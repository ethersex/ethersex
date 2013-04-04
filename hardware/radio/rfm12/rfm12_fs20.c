/*
 * Copyright (c) 2012-13 Erik Kunze <ethersex@erik-kunze.de>
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

#include <util/delay.h>
#include <util/parity.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/bool.h"
#include "core/heartbeat.h"
#include "core/periodic.h"
#ifdef RFM12_ASK_FS20_SYSLOG
#include "core/util/byte2hex.h"
#include "protocols/syslog/syslog.h"
#endif

#include "rfm12.h"
#include "rfm12_ask.h"
#include "rfm12_fs20.h"

#ifdef RFM12_ASK_FS20_RX_SUPPORT

/* FS20 read routines use timer 2 */
/* Determine best prescaler depending on F_CPU */
#define FS20_SILENCE         4100

#define FS20_MAX_OVERFLOW    255UL
#if (F_CPU/1000000*FS20_SILENCE) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       1UL
#elif (F_CPU/1000000*FS20_SILENCE/8) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       8UL
#elif (F_CPU/1000000*FS20_SILENCE/64) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       64UL
#elif (F_CPU/1000000*FS20_SILENCE/128) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       128UL
#elif (F_CPU/1000000*FS20_SILENCE/256) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       256UL
#elif (F_CPU/1000000*FS20_SILENCE/1024) < FS20_MAX_OVERFLOW
#define FS20_PRESCALER       1024UL
#else
#error F_CPU to large
#endif

/* Scaling time to enable 8bit arithmetic */
#define US2TICK(x)           (uint8_t)(F_CPU/1000000*(x)/FS20_PRESCALER)
#define TICK2US(x)           (uint16_t)(1000000*FS20_PRESCALER*(x)/F_CPU)

#define FS20_TIMER_INT_ON    TC2_INT_COMPARE_ON
#define FS20_TIMER_INT_OFF   TC2_INT_COMPARE_OFF
#define FS20_TIMER_INT_CLR   TC2_INT_COMPARE_CLR
#define FS20_TIMER_CNT_CURR  TC2_COUNTER_CURRENT
#define FS20_TIMER_CNT_COMP  TC2_COUNTER_COMPARE
#define ticks                TC1_COUNTER_CURRENT
#define ticks_per_second     CLOCK_SECONDS

/* culfw decoding routines */
#include "rfm12_fs20_lib.c"

#define FIFO_SIZE            8
#define FIFO_NEXT(x)         (((x)+1)&(FIFO_SIZE-1))

typedef struct
{
  uint8_t read;
  uint8_t write;
  fs20_data_t buffer[FIFO_SIZE];
} fs20_fifo_t;

static fs20_fifo_t fs20_rx_fifo;

static void
activate_ask_receiver(void)
{
  /* turn on external filter again */
  rfm12_prologue(RFM12_MODULE_FS20);
  rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER | RFM12_PWRMGT_EBB);
  rfm12_trans(RFM12_CMD_DATAFILTER & ~0x0008);
  rfm12_epilogue();
}
#endif /* RFM12_ASK_FS20_RX_SUPPORT */


#ifdef RFM12_ASK_FS20_TX_SUPPORT
static void
send_bits(uint16_t data, uint8_t bits)
{
  if (bits == 8)
  {
    ++bits;
    data = (data << 1) | parity_even_bit(data);
  }

  for (uint16_t mask = (uint16_t) _BV(bits - 1); mask; mask >>= 1)
  {
    /* Timing values empirically obtained, and used to adjust for on/off
     * delay in the RFM12. The actual on-the-air bit timing we're after is
     * 600/600us for 1 and 400/400us for 0 - but to achieve that the RFM12B
     * needs to be turned on a bit longer and off a bit less. In addition
     * there is about 25 uS overhead in sending the on/off command over SPI.
     * With thanks to JGJ Veken for his help in getting these values right.
     */
    uint16_t width = data & mask ? 600 : 400;
    rfm12_ask_trigger(1, width + 150);
    rfm12_ask_trigger(0, width - 200);
  }
}

static void
fs20_send_internal(uint8_t fht, uint16_t house, uint8_t addr, uint8_t cmd,
                   uint8_t data)
{
  for (uint8_t i = 3; i; i--)
  {
    uint8_t sum = fht ? 0x0c : 0x06;

#ifdef STATUSLED_RFM12_TX_SUPPORT
    PIN_SET(STATUSLED_RFM12_TX);
#endif

    rfm12_prologue(RFM12_MODULE_FS20);

    send_bits(1, 13);
    send_bits(HI8(house), 8);
    sum += HI8(house);
    send_bits(LO8(house), 8);
    sum += LO8(house);
    send_bits(addr, 8);
    sum += addr;
    send_bits(cmd, 8);
    sum += cmd;
    if (cmd & 0x20)
    {
      send_bits(data, 8);
      sum += data;
    }
    send_bits(sum, 8);
    send_bits(0, 1);

    rfm12_epilogue();

#ifdef STATUSLED_RFM12_TX_SUPPORT
    PIN_CLEAR(STATUSLED_RFM12_TX);
#endif

    _delay_ms(10);
  }

#ifdef RFM12_ASK_FS20_RX_SUPPORT
  activate_ask_receiver();
#endif
}

void
rfm12_fs20_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(FALSE, house, addr, cmd, data);
}

#ifdef RFM12_ASK_FHT_SUPPORT
void
rfm12_fht_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(TRUE, house, addr, cmd, data);
}
#endif /* RFM12_ASK_FHT_SUPPORT */
#endif /* RFM12_ASK_FS20_TX_SUPPORT */

#ifdef RFM12_ASK_FS20_RX_SUPPORT
ISR(TC2_VECTOR_COMPARE)
{
  rfm12_fs20_lib_rx_timeout();
}

ISR(RFM12_FS20INT_VECTOR)
{
  uint8_t count = TC2_COUNTER_CURRENT;
  uint8_t is_raising_edge = PIN_HIGH(RFM12_FS20IN);
#ifdef STATUSLED_RFM12_RX_SUPPORT
  if (is_raising_edge)
    PIN_SET(STATUSLED_RFM12_RX);
  else
    PIN_CLEAR(STATUSLED_RFM12_RX);
#endif
  rfm12_fs20_lib_rx_level_changed(count, is_raising_edge);
}

static void
rfm12_fs20_init_rx(void)
{
  rfm12_fs20_lib_init();

  /* configure timer2 for receiving fs20 */
  TC2_COUNTER_CURRENT = 0;
#if FS20_PRESCALER == 1UL
  TC2_PRESCALER_1;
#elif FS20_PRESCALER == 8UL
  TC2_PRESCALER_8;
#elif FS20_PRESCALER == 64UL
  TC2_PRESCALER_64;
#elif FS20_PRESCALER == 128UL
  TC2_PRESCALER_128;
#elif FS20_PRESCALER == 256UL
  TC2_PRESCALER_256;
#elif FS20_PRESCALER == 1024UL
  TC2_PRESCALER_1024;
#endif
  TC2_MODE_OFF;
  TC2_OUTPUT_COMPARE_NONE;
  TC2_INT_COMPARE_OFF;
  TC2_INT_OVERFLOW_OFF;

#ifdef DEBUG_ASK_FS20
  debug_printf("rfm12_fs20 prescaler %u, tick %u us\n",
               (uint16_t) FS20_PRESCALER, (uint16_t) TICK2US(1));
#endif

  /* Initialize Interrupt */
  _EIMSK |= _BV(RFM12_FS20INT_PIN);
  _EICRA = (uint8_t) ((_EICRA & ~RFM12_FS20INT_ISCMASK) | RFM12_FS20INT_ISC);

  activate_ask_receiver();

#ifdef STATUSLED_RFM12_RX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_RX);
#endif
}
#endif /* RFM12_ASK_FS20_RX_SUPPORT */

void
rfm12_fs20_init(void)
{
  /* wait until POR done */
  for (uint8_t i = 15; i; i--)
    _delay_ms(10);

  rfm12_prologue(RFM12_MODULE_FS20);

  rfm12_trans(RFM12_CMD_LBDMCD | 0xE0);
  rfm12_trans(RFM12_CMD_CFG | RFM12_BAND_868 | RFM12_XTAL_135PF);
  rfm12_trans(RFM12_CMD_FIFORESET);
  rfm12_trans(RFM12_CMD_WAKEUP);
  rfm12_trans(RFM12_CMD_DUTYCYCLE);
  rfm12_trans(RFM12_CMD_AFC | RFM12_AFC_AUTO_ONCE | RFM12_AFC_LIMIT_4 |
              RFM12_AFC_OE | RFM12_AFC_EN);

#ifdef DEBUG
  uint16_t result = rfm12_trans(RFM12_CMD_STATUS);
  RFM12_DEBUG("rfm12_fs20/init: %x", result);
#endif

  rfm12_setfreq(RFM12FREQ(RFM12_FREQ_868300));
  /* bandwidth, gain, drssi */
  rfm12_setbandwidth(1, 2, 1);

  rfm12_epilogue();

#ifdef STATUSLED_RFM12_TX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_TX);
#endif

#ifdef RFM12_ASK_FS20_RX_SUPPORT
  rfm12_fs20_init_rx();
#endif
}

#ifdef RFM12_ASK_FS20_RX_SUPPORT
void
rfm12_fs20_process(void)
{
  fs20_data_t fs20_data;
  if (rfm12_fs20_lib_process(&fs20_data))
  {
#ifdef RFM12_ASK_FS20_SYSLOG
    uint8_t buf[2 * FS20_MAXMSG + 2];
    buf[0] = fs20_data.datatype;
    uint8_t count = fs20_data.count;
    if (fs20_data.nibble)
      count--;
    uint8_t i = 1;
    for (uint8_t j = 0; j < count; j++)
      i += byte2hex(fs20_data.data[j], &buf[i]);
    if (nibble)
    {
      byte2hex(fs20_data.data[count], &buf[i]);
      buf[i] = buf[i + 1];
      i++;
    }
    buf[i] = '\0';
    syslog_send(buf);
#endif
    uint8_t tmphead = FIFO_NEXT(fs20_rx_fifo.write);
    if (tmphead != fs20_rx_fifo.read)
    {
      fs20_rx_fifo.buffer[tmphead] = fs20_data;
      fs20_rx_fifo.write = tmphead;
    }
  }
}

fs20_data_t *
rfm12_fs20_read(void)
{
  return (fs20_rx_fifo.read == fs20_rx_fifo.write ?
          0 : &fs20_rx_fifo.buffer[fs20_rx_fifo.read =
                                   FIFO_NEXT(fs20_rx_fifo.read)]);
}
#endif /* RFM12_ASK_FS20_RX_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/radio/rfm12/rfm12_fs20.h)
  init(rfm12_fs20_init)
  ifdef(`conf_RFM12_ASK_FS20_RX', `mainloop(rfm12_fs20_process)')
*/
