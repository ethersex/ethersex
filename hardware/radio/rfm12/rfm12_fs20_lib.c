/*
 * Copyright (c) Rudolg Koenig (culfw)
 * Copyright (c) 2012-15 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdint.h>
#include <util/parity.h>

#include "config.h"
#include "core/bit-macros.h"

#include "rfm12.h"
#include "rfm12_fs20_lib.h"

/*
 * Source code taken from http://culfw.de and adapted to Ethersex
 */

/*
 * With a CUL measured RF timings, in us, high/low sum
 *          Bit zero        Bit one
 * KS300:  854/366 1220    366/854 1220
 * HRM:    992/448 1440    528/928 1456
 * EM:     400/320  720    432/784 1216
 * S300:   784/368 1152    304/864 1168
 * FHT:    362/368  730    565/586 1151
 * FS20:   376/357  733    592/578 1170
 */

/* tolerated diff to previous/avg high/low/total */
#define TDIFF          US2TICK(200)
/* End of message */
#define SILENCE        4000

#define STATE_RESET    0
#define STATE_INIT     1
#define STATE_SYNC     2
#define STATE_COLLECT  3
#define STATE_HMS      4
#define STATE_ESA      5

/* public prototypes */
#define RCV_BUCKETS    4

#define FHT_ACTUATOR   0x00
#define FHT_ACK        0x4B
#define FHT_CAN_XMIT   0x53
#define FHT_CAN_RCV    0x54
#define FHT_ACK2       0x69
#define FHT_START_XMIT 0x7D
#define FHT_END_XMIT   0x7E
#define FHT_MINUTE     0x64
#define FHT_HOUR       0x63

#define TYPE_EM        'E'
#define TYPE_HMS       'H'
#define TYPE_FHT       'T'
#define TYPE_FS20      'F'
#define TYPE_KS300     'K'
#define TYPE_HRM       'R'      /* Hoermann */
#define TYPE_ESA       'S'
#define TYPE_TX3       't'

#define REP_KNOWN      _BV(0)
#define REP_REPEATED   _BV(1)
#define REP_BITS       _BV(2)
#define REP_MONITOR    _BV(3)
#define REP_BINTIME    _BV(4)
#define REP_FHTPROTO   _BV(6)

#define DC             putchar
#define DS_P           printf_P
#define DU(a,b)        printf_P(PSTR("%5u"),a)
#define DH(a,b)        printf_P(PSTR("%X"),a)
#define DH2(a)         printf_P(PSTR("%02X"),a)
#define DNL()          putchar('\n')


uint8_t rx_report = REP_KNOWN;  /* global verbose / output-filter */

typedef struct
{
  uint8_t hightime, lowtime;
} wave_t;

/* One bucket to collect the "raw" bits */
typedef struct
{
  uint8_t state, byteidx, sync, bitidx;
  uint8_t data[FS20_MAXMSG];    /* contains parity and checksum, but no sync */
  wave_t zero, one;
} bucket_t;

static bucket_t bucket_array[RCV_BUCKETS];
static uint8_t bucket_in;                       /* Pointer to the in (terrupt) queue */
static uint8_t bucket_out;                      /* Pointer to the out (analyze) queue */
static uint8_t bucket_nrused;                   /* Number of unprocessed buckets */
static uint8_t oby, obuf[FS20_MAXMSG], nibble;  /* parity-stripped output */
static uint8_t roby, robuf[FS20_MAXMSG];        /* for Repeat check: buffer and time */
static uint32_t reptime;
static uint8_t hightime, lowtime;

static void addbit(bucket_t * b, uint8_t bit);
static void delbit(bucket_t * b);
static uint8_t wave_equals(wave_t * a, uint8_t htime, uint8_t ltime);


/* FS20 / FHT */
static uint8_t
cksum1(uint8_t s, uint8_t * buf, uint8_t len)
{
  while (len)
    s += buf[--len];
  return s;
}

/* EM */
static uint8_t
cksum2(uint8_t * buf, uint8_t len)
{
  uint8_t s = 0;
  while (len)
    s ^= buf[--len];
  return s;
}

/* KS300 */
static uint8_t
cksum3(uint8_t * buf, uint8_t len)
{
  uint8_t x = 0, y = 5, cnt = 0;
  while (len)
  {
    uint8_t d = buf[--len];
    x ^= HI4(d);
    y += HI4(d);
    if (!nibble || cnt)
    {
      x ^= LO4(d);
      y += LO4(d);
    }
    cnt++;
  }
  y += x;
  return (y << 4) | x;
}

static uint8_t
analyze(bucket_t * b, uint8_t t)
{
  uint8_t cnt = 0, max, iby = 0;
  int8_t ibi = 7, obi = 7;

  nibble = 0;
  oby = 0;
  max = b->byteidx * 8 + (7 - b->bitidx);
  obuf[0] = 0;
  while (cnt++ < max)
  {
    uint8_t bit = (b->data[iby] & _BV(ibi)) ? 1 : 0;    /* Input bit */
    if (ibi-- == 0)
    {
      iby++;
      ibi = 7;
    }

    if (t == TYPE_KS300 && obi == 3)
    {                           /* nibble check */
      if (!nibble)
      {
        if (!bit)
          return 0;
        nibble = !nibble;
        continue;
      }
      nibble = !nibble;
    }

    if (obi == -1)
    {                           /* next byte */
      if (t == TYPE_FS20)
      {
        if (parity_even_bit(obuf[oby]) != bit)
          return 0;
      }
      if (t == TYPE_EM || t == TYPE_KS300)
      {
        if (!bit)
          return 0;
      }
      obuf[++oby] = 0;
      obi = 7;

    }
    else
    {                           /* Normal bits */
      if (bit)
      {
        if (t == TYPE_FS20)
          obuf[oby] |= _BV(obi);
        if (t == TYPE_EM || t == TYPE_KS300)    /* LSB */
          obuf[oby] |= _BV(7 - obi);
      }
      obi--;
    }
  }
  if (cnt <= max)
    return 0;
  else if (t == TYPE_EM && obi == -1)   /* missing last stopbit */
    oby++;
  else if (nibble)              /* half byte msg */
    oby++;

  if (oby == 0)
    return 0;
  return 1;
}

typedef struct
{
  uint8_t *data;
  uint8_t byte, bit;
} input_t;

static uint8_t
getbit(input_t * in)
{
  uint8_t bit = (in->data[in->byte] & _BV(in->bit)) ? 1 : 0;
  if (in->bit-- == 0)
  {
    in->byte++;
    in->bit = 7;
  }
  return bit;
}

static uint8_t
getbits(input_t * in, uint8_t nbits, uint8_t msb)
{
  uint8_t ret = 0, i;
  for (i = 0; i < nbits; i++)
  {
    if (getbit(in))
      ret = ret | _BV(msb ? nbits - i - 1 : i);
  }
  return ret;
}

static uint8_t
analyze_hms(bucket_t * b)
{
  input_t in;
  in.byte = 0;
  in.bit = 7;
  in.data = b->data;

  oby = 0;
  if (b->byteidx * 8 + (7 - b->bitidx) < 69)
    return 0;

  uint8_t crc = 0;
  for (oby = 0; oby < 6; oby++)
  {
    obuf[oby] = getbits(&in, 8, 0);
    if (parity_even_bit(obuf[oby]) != getbit(&in))
      return 0;
    if (getbit(&in))
      return 0;
    crc = crc ^ obuf[oby];
  }

  /* Read crc */
  uint8_t CRC = getbits(&in, 8, 0);
  if (parity_even_bit(CRC) != getbit(&in))
    return 0;
  if (crc != CRC)
    return 0;
  return 1;
}


#ifdef RFM12_ASK_ESA_SUPPORT
static uint8_t
analyze_esa(bucket_t * b)
{
  input_t in;
  in.byte = 0;
  in.bit = 7;
  in.data = b->data;

  oby = 0;

  if (b->state != STATE_ESA)
    return 0;

  if ((b->byteidx * 8 + (7 - b->bitidx)) != 144)
    return 0;

  uint8_t salt = 0x89;
  uint16_t crc = 0xf00f;

  for (oby = 0; oby < 15; oby++)
  {
    uint8_t byte = getbits(&in, 8, 1);

    crc += byte;

    obuf[oby] = byte ^ salt;
    salt = byte + 0x24;
  }

  obuf[oby] = getbits(&in, 8, 1);
  crc += obuf[oby];
  obuf[oby++] ^= 0xff;

  crc -= (getbits(&in, 8, 1) << 8);
  crc -= getbits(&in, 8, 1);

  if (crc)
    return 0;

  return 1;
}
#endif

#ifdef RFM12_ASK_TX3_SUPPORT
static uint8_t
analyze_TX3(bucket_t * b)
{
  input_t in;
  in.byte = 0;
  in.bit = 7;
  in.data = b->data;
  uint8_t n, crc = 0;

  if (b->byteidx != 4 || b->bitidx != 1)
    return 0;

  for (oby = 0; oby < 4; oby++)
  {
    if (oby == 0)
    {
      n = 0x80 | getbits(&in, 7, 1);
    }
    else
    {
      n = getbits(&in, 8, 1);
    }
    crc = crc + HI4(n) + LO4(n);
    obuf[oby] = n;
  }

  obuf[oby] = getbits(&in, 7, 1) << 1;
  crc = LO4(crc + HI4(obuf[oby]));
  oby++;

  if (HI4(crc) != 0 || HI4(obuf[0]) != 0xA)
    return 0;

  return 1;
}
#endif

int
rfm12_fs20_lib_process(fs20_data_t * fs20_data_p)
{
  uint8_t valid = 0;
  uint8_t datatype = 0;
  bucket_t *b;

  if (lowtime)
  {
#ifdef DEBUG_ASK_FS20
    if (rx_report & REP_MONITOR)
    {
      DC('r');
      if (rx_report & REP_BINTIME)
        DU(hightime, 2);
      DC('f');
      if (rx_report & REP_BINTIME)
        DU(lowtime, 2);
    }
#endif
    lowtime = 0;
  }

  if (bucket_nrused == 0)
    return valid;

  ACTIVITY_LED_RFM12_RX;

  b = bucket_array + bucket_out;

#ifdef RFM12_ASK_ESA_SUPPORT
  if (analyze_esa(b))
    datatype = TYPE_ESA;
#endif

  if (!datatype && analyze(b, TYPE_FS20))
  {
    oby--;                      /* Separate the checksum byte */
    uint8_t fs_csum = cksum1(6, obuf, oby);
    if (fs_csum == obuf[oby] && oby >= 4)
    {
      datatype = TYPE_FS20;
    }
    else if (fs_csum + 1 == obuf[oby] && oby >= 4)
    {                           /* Repeater */
      datatype = TYPE_FS20;
      obuf[oby] = fs_csum;      /* do not report if we get both */
    }
    else if (cksum1(12, obuf, oby) == obuf[oby] && oby >= 4)
    {
      datatype = TYPE_FHT;
    }
    else
    {
      datatype = 0;
    }
  }

  if (!datatype && analyze(b, TYPE_EM))
  {
    oby--;
    if (oby == 9 && cksum2(obuf, oby) == obuf[oby])
      datatype = TYPE_EM;
  }

  if (!datatype && analyze_hms(b))
    datatype = TYPE_HMS;

#ifdef RFM12_ASK_TX3_SUPPORT
  if (!datatype && analyze_TX3(b))
    datatype = TYPE_TX3;
#endif

  if (!datatype)
  {
    /* As there is no last rise, we have to add the last bit by hand */
    addbit(b, wave_equals(&b->one, hightime, b->one.lowtime));
    if (analyze(b, TYPE_KS300))
    {
      oby--;
      if (cksum3(obuf, oby) == obuf[oby - nibble])
        datatype = TYPE_KS300;
    }
    if (!datatype)
      delbit(b);
  }

  if (datatype && (rx_report & REP_KNOWN))
  {
    uint8_t isrep = 0;
    if (!(rx_report & REP_REPEATED))
    {                           /* Filter repeated messages */

      /* compare the data */
      if (roby == oby)
      {
        for (roby = 0; roby < oby; roby++)
          if (robuf[roby] != obuf[roby])
            break;

        if (roby == oby)
        {
          uint32_t elapsed_time = FS20_TIMER_TICKS;
          if (elapsed_time < reptime)
            elapsed_time += FS20_TIMER_TICKSPERSEC;
          elapsed_time -= reptime;

          /* ~0.2 sec */
          if (elapsed_time < (FS20_TIMER_TICKSPERSEC / 5))
            isrep = 1;
        }
      }

      /* save the data */
      for (roby = 0; roby < oby; roby++)
        robuf[roby] = obuf[roby];
      reptime = FS20_TIMER_TICKS;
    }

    if (datatype == TYPE_FHT && !(rx_report & REP_FHTPROTO) &&
        oby > 4 &&
        (obuf[2] == FHT_ACK || obuf[2] == FHT_ACK2 ||
         obuf[2] == FHT_CAN_XMIT || obuf[2] == FHT_CAN_RCV ||
         obuf[2] == FHT_START_XMIT || obuf[2] == FHT_END_XMIT ||
         (obuf[3] & 0x70) == 0x70))
      isrep = 1;

    if (!isrep)
    {
      fs20_data_p->datatype = datatype;
      fs20_data_p->count = oby;
      fs20_data_p->nibble = nibble;
      for (uint8_t i = 0; i < oby; i++)
        fs20_data_p->data[i] = obuf[i];
#ifdef DEBUG_ASK_FS20
      DC(datatype);
      if (nibble)
        oby--;
      for (uint8_t i = 0; i < oby; i++)
        DH2(obuf[i]);
      if (nibble)
        DH(obuf[oby] & 0xf, 1);
      DNL();
#endif
      valid = 1;
    }
  }

#ifdef DEBUG_ASK_FS20
  if (rx_report & REP_BITS)
  {
    DC('p');
    DU(b->state, 2);
    DU(TICK2US(b->zero.hightime), 5);
    DU(TICK2US(b->zero.lowtime), 5);
    DU(TICK2US(b->one.hightime), 5);
    DU(TICK2US(b->one.lowtime), 5);
    DU(b->sync, 3);
    DU(b->byteidx, 3);
    DU(7 - b->bitidx, 2);
    DC(' ');
    if (b->bitidx != 7)
      b->byteidx++;

    for (uint8_t i = 0; i < b->byteidx; i++)
      DH2(b->data[i]);
    DNL();
  }
#endif

  b->state = STATE_RESET;
  bucket_nrused--;
  bucket_out++;
  if (bucket_out == RCV_BUCKETS)
    bucket_out = 0;

  return valid;
}

static void
reset_input(void)
{
  FS20_TIMER_STOP;
  bucket_array[bucket_in].state = STATE_RESET;
}

static void
rfm12_fs20_lib_rx_timeout(void)
{
  FS20_TIMER_STOP;              /* Disable "us" */

#ifdef DEBUG_ASK_FS20
  if (rx_report & REP_MONITOR)
    DC('.');
#endif

  if (bucket_array[bucket_in].state < STATE_COLLECT ||
      bucket_array[bucket_in].byteidx < 2)
  {                             /* false alarm */
    reset_input();
    return;
  }

  if (bucket_nrused + 1 == RCV_BUCKETS)
  {                             /* each bucket is full: reuse the last */
#ifdef DEBUG_ASK_FS20
    if (rx_report & REP_BITS)
      DS_P(PSTR("BOVF\r\n"));   /* Bucket overflow */
#endif

    reset_input();
  }
  else
  {
    bucket_nrused++;
    bucket_in++;
    if (bucket_in == RCV_BUCKETS)
      bucket_in = 0;
  }
}

static uint8_t
wave_equals(wave_t * a, uint8_t htime, uint8_t ltime)
{
  int16_t dlow = a->lowtime - ltime;
  int16_t dhigh = a->hightime - htime;
  int16_t dcomplete = (a->lowtime + a->hightime) - (ltime + htime);
  if (dlow < TDIFF && dlow > -TDIFF &&
      dhigh < TDIFF && dhigh > -TDIFF &&
      dcomplete < TDIFF && dcomplete > -TDIFF)
    return 1;
  return 0;
}

static uint8_t
makeavg(uint8_t i, uint8_t j)
{
  return (i + i + i + j) / 4;
}

static void
addbit(bucket_t * b, uint8_t bit)
{
  if (b->byteidx >= sizeof(b->data))
  {
    reset_input();
    return;
  }
  if (bit)
    b->data[b->byteidx] |= _BV(b->bitidx);

  if (b->bitidx-- == 0)
  {                             /* next byte */
    b->bitidx = 7;
    b->data[++b->byteidx] = 0;
  }
}

static void
delbit(bucket_t * b)
{
  if (b->bitidx++ == 7)
  {                             /* prev byte */
    b->bitidx = 0;
    b->byteidx--;
  }
}

static void
rfm12_fs20_lib_rx_level_changed(uint8_t c, uint8_t is_raising_edge)
{
  bucket_t *b = bucket_array + bucket_in;       /* where to fill in the bit */

  if (b->state == STATE_HMS)
  {
    if (c < US2TICK(750))
      return;
    if (c > US2TICK(1250))
    {
      reset_input();
      return;
    }
  }

#ifdef RFM12_ASK_ESA_SUPPORT
  if (b->state == STATE_ESA)
  {
    if (c < US2TICK(375))
      return;
    if (c > US2TICK(625))
    {
      reset_input();
      return;
    }
  }
#endif

  /* Falling edge */
  FS20_TIMER_INT_CLR;
  if (!is_raising_edge)
  {
    if ((b->state == STATE_HMS)
#ifdef RFM12_ASK_ESA_SUPPORT
        || (b->state == STATE_ESA)
#endif
      )
    {
      addbit(b, 1);
      FS20_TIMER_RESTART;
    }
    hightime = c;
    return;
  }

  lowtime = c - hightime;
  FS20_TIMER_RESTART;           /* restart timer */
  if ((b->state == STATE_HMS)
#ifdef RFM12_ASK_ESA_SUPPORT
      || (b->state == STATE_ESA)
#endif
    )
  {
    addbit(b, 0);
    return;
  }

  if (b->state == STATE_RESET)
  {                             /* first sync bit, cannot compare yet */
  retry_sync:
    if (hightime > US2TICK(1600) || lowtime > US2TICK(1600))
      return;

    b->zero.hightime = hightime;
    b->zero.lowtime = lowtime;
    b->sync = 1;
    b->state = STATE_SYNC;
  }
  else if (b->state == STATE_SYNC)
  {                             /* sync: lots of zeroes */
    if (wave_equals(&b->zero, hightime, lowtime))
    {
      b->zero.hightime = makeavg(b->zero.hightime, hightime);
      b->zero.lowtime = makeavg(b->zero.lowtime, lowtime);
      b->sync++;
    }
    else if (b->sync >= 4)
    {                           /* the one bit at the end of the 0-sync */
      uint16_t to = SILENCE;

      if (b->sync >= 12 &&
          (b->zero.hightime + b->zero.lowtime) > US2TICK(1600))
      {
        b->state = STATE_HMS;
      }
#ifdef RFM12_ASK_ESA_SUPPORT
      else if (b->sync >= 10 &&
               (b->zero.hightime + b->zero.lowtime) < US2TICK(600))
      {
        b->state = STATE_ESA;

        to = 1000;
      }
#endif
      else
      {
        b->state = STATE_COLLECT;
      }

      b->one.hightime = hightime;
      b->one.lowtime = lowtime;
      b->byteidx = 0;
      b->bitidx = 7;
      b->data[0] = 0;

      FS20_TIMER_TIMEOUT(to);
      FS20_TIMER_START ;        /* On timeout analyze the data */
    }
    else
    {                           /* too few sync bits */
      b->state = STATE_RESET;
      goto retry_sync;
    }
  }
  else
  {                             /* STATE_COLLECT */
    if (wave_equals(&b->one, hightime, lowtime))
    {
      addbit(b, 1);
      b->one.hightime = makeavg(b->one.hightime, hightime);
      b->one.lowtime = makeavg(b->one.lowtime, lowtime);
    }
    else if (wave_equals(&b->zero, hightime, lowtime))
    {
      addbit(b, 0);
      b->zero.hightime = makeavg(b->zero.hightime, hightime);
      b->zero.lowtime = makeavg(b->zero.lowtime, lowtime);
    }
    else
    {
      reset_input();
    }
  }
}

static void
rfm12_fs20_lib_init(void)
{
  for (uint8_t i = 1; i < RCV_BUCKETS; i++)
    bucket_array[i].state = STATE_RESET;
  FS20_TIMER_TIMEOUT(SILENCE);
  FS20_TIMER_INIT;
}
