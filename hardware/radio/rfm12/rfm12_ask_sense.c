/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2010-14 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <avr/interrupt.h>
#include <string.h>

#include "config.h"
#include "rfm12_ask.h"

/*
  Theory of operation:
 -----------------------
  The ASK signal we would like to sense looks something like this:

               1    2    3    4
               _____     _____          __________
  NNNNNNNNNNNNN     NNNNN     NNNNNNNNNN          NNN...

      result:   0    0    0       1         1   ....

  Right at the beginning there's a (very) long phase of noise, i.e. no
  transmitter is active.  The transmission always begings with a
  sending phase, i.e. there is no noise anymore somewhen.  If the
  transmitter is disabled again, there's noise again, and so on.

  We sense the signal by continuously updating the variable LAST_NOISE_TS in
  the noise phases (until 1).  During a TX-active phase there won't be any
  interrupts, i.e. we're not updating that variable any longer.

  As soon as the transmitter is disabled again, we'll get another
  interrupt (2) and can calculate the time delta.  If we've successfully
  detected a TX-active phase (not just yet another noise spike) we can lock
  the receiver and further read the signal ...

  At any _ to N transision (2,4) we'll update LAST_TX_TS.
  From 2 to 3 we again update LAST_NOISE_TS.

  At 4 we can calculate the length of the TX-active-phase
  (NOW - LAST_NOISE_TX) and the length
  of the noise-phase (LAST_NOISE_TX - LAST_TX_TS).
*/

#define PRESCALER         256
#define NS_PER_TICK       (PRESCALER * 1000000UL / F_CPU * 1000)
#define US_TO_TICKS(n)    ((n) * 1000UL / NS_PER_TICK)

/* Every TX-period shorter that 500us is considered part of a noise phase. */
#define MIN_TICKS         US_TO_TICKS(500)

/* Every phase shorter than 950us is considered `short', i.e. a ZERO.
   Phases longer are considered long, i.e. a ONE. */
#define DEF_LIMIT_TICKS   US_TO_TICKS(950)

#define WAIT_SAMPLES_NUM  16
#define LIMIT_TICKS       (samples_num < WAIT_SAMPLES_NUM ? DEF_LIMIT_TICKS : samples_limit)

/* Timeout */
#define TIMEOUT_TICKS     US_TO_TICKS(2000)


static uint8_t last_noise_ts;
static uint8_t last_tx_ts;

static volatile struct
{
  uint8_t overflow;
} bits;

/* Calculate delta between new and old, considering that the values
   overflow at 256.*/
#define DELTA(n,o)        (uint8_t)(bits.overflow == 0 ? ((n) - (o)) : (256 - (o) + (n)))

#define TICKS_TO_BIT(n)   ((n) > LIMIT_TICKS ? 1 : 0)


#ifdef DEBUG_ASK_SENSE
#include "core/debug.h"
#define ASKDEBUG(a...)   debug_printf(a)
#define ASKDEBUGCHAR(a)  debug_putchar((char)(a))
#else
#define ASKDEBUG(a...)
#define ASKDEBUGCHAR(a)
#endif /* DEBUG_ASK_SENSE */



/* Number of samples to gather, i.e. use for calibration. */
static uint8_t samples_min, samples_max, samples_num, samples_limit;

static void
samples_learn(const uint8_t new_sample)
{
  if (samples_num > WAIT_SAMPLES_NUM)
    return;                     /* Gathered enough samples. */

  if (samples_num == 0)
    samples_min = samples_max = new_sample;

  else
  {
    if (new_sample < samples_min)
      samples_min = new_sample;
    if (new_sample > samples_max)
      samples_max = new_sample;
  }

  if (++samples_num == WAIT_SAMPLES_NUM)
  {
    samples_limit = (uint8_t) ((samples_min + samples_max) / 2);
    ASKDEBUG("min=%d, max=%d\n", samples_min, samples_max);
  }
}


static uint8_t ask_buf[8];
static uint8_t ask_buf_bits;

#define ASK_BUF_STORE(b)  { if (b) ask_sense_store_bit(ask_buf_bits); \
                            ask_buf_bits++; }

static void
ask_sense_clear_bits(void)
{
  memset(ask_buf, 0, sizeof(ask_buf));
  ask_buf_bits = 0;
}

static void
ask_sense_store_bit(const uint8_t bitcount)
{
  uint8_t byte = (bitcount / 8) % sizeof(ask_buf);
  uint8_t bit = bitcount % 8;
  ask_buf[byte] |= (uint8_t) _BV(bit);
}

void
rfm12_ask_sense_start(void)
{
  ASKDEBUG("initializing.\n");

  /* Initialize Timer0, prescaler 1/256 */
  TC0_PRESCALER_256;
  TC0_INT_OVERFLOW_ON;

  /* Initialize Interrupt */
  _EIMSK |= _BV(RFM12_ASKINT_PIN);
  _EICRA = (uint8_t) ((_EICRA & ~RFM12_ASKINT_ISCMASK) | RFM12_ASKINT_ISC);

  last_noise_ts = TC0_COUNTER_CURRENT;
  ask_sense_clear_bits();
  samples_num = 0;

  rfm12_ask_external_filter_init();
}


static void
ask_sense_decode_tevion(void)
{
#ifdef DEBUG_ASK_SENSE
  uint8_t code1 = (uint8_t) (ask_buf[3] << 1) | (uint8_t) (ask_buf[4] >> 7);
  uint8_t code2 = (uint8_t) (ask_buf[4] << 1) | (uint8_t) (ask_buf[5] >> 7);
  ASKDEBUG("rfm12 tevion %d,%d,%d %d,%d 99 4\n",
           ask_buf[0], ask_buf[1], ask_buf[2], code1, code2);
#endif /* DEBUG_ASK_SENSE */
}


static void
ask_sense_try_decode(void)
{
  if (ask_buf_bits == 41)
    ask_sense_decode_tevion();
  else
    ASKDEBUG("try_decode: unknown code.\n");

  ask_sense_clear_bits();
}


ISR(TC0_VECTOR_OVERFLOW)
{
  if (bits.overflow && ask_buf_bits)
  {
    bits.overflow = 0;

    ASKDEBUGCHAR('t');
    ASKDEBUGCHAR(10);

    ask_sense_try_decode();     /* Resets ask_buf_bits. */
    return;
  }

  bits.overflow = 1;
}


ISR(RFM12_ASKINT_VECTOR)
{
  uint8_t ts = TC0_COUNTER_CURRENT;     /* Get current timestamp. */
  uint8_t delta = DELTA(ts, last_noise_ts);

  if (delta > TIMEOUT_TICKS && ask_buf_bits)
  {
    ASKDEBUGCHAR('T');
    ASKDEBUGCHAR('\n');

    ask_sense_try_decode();     /* Resets ask_buf_bits. */
    bits.overflow = 0;
  }

  if (delta < MIN_TICKS || delta > TIMEOUT_TICKS)
  {
    /* Within noise period, update last_noise_ts and we're done. */
    last_noise_ts = ts;
    return;
  }

  uint8_t bit;

  /* We've detected the end of a TX-active phase... */
  if (ask_buf_bits)
  {
    /* ... and it's not the first bit we're receiving. */
    uint8_t delta2 = DELTA(last_noise_ts, last_tx_ts);

    samples_learn(delta2);
    bit = TICKS_TO_BIT(delta2);

    ASKDEBUGCHAR('0' + bit);
    ASK_BUF_STORE(bit);
  }

  samples_learn(delta);
  bit = TICKS_TO_BIT(delta);

  ASKDEBUGCHAR('0' + bit);
  ASK_BUF_STORE(bit);           /* increments ask_buf_bits. */

  bits.overflow = 0;
  last_noise_ts = ts;
  last_tx_ts = ts;
}
