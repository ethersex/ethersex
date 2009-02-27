/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "../config.h"
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

#ifdef RFM12_ASK_SENSING_SUPPORT

#define PRESCALER 256
#define NS_PER_TICK     (PRESCALER * 1000000UL / F_CPU * 1000)
#define US_TO_TICKS(n)  ((n) * 1000UL / NS_PER_TICK)

/* Every TX-period shorter that 350us is considered part of a noise phase. */
#define MIN_TICKS   US_TO_TICKS(350)

/* Every phase shorter than 950us is considered `short', i.e. a ZERO.
   Phases longer are considered long, i.e. a ONE. */
#define LIMIT_TICKS US_TO_TICKS(950)

/* Timeout */
#define TIMEOUT_TICKS US_TO_TICKS(1600)


static uint8_t last_noise_ts;
static uint8_t last_tx_ts;
static uint8_t locked_signal;

/* Calculate delta between new and old, considering that the values
   overflow at 256.*/
#define DELTA(n,o) (((n) >= (o)) ? ((n) - (o)) : (256 - (o) + (n)))

#define TICKS_TO_BIT(n) ((n) > LIMIT_TICKS ? 1 : 0)


#ifdef DEBUG_ASK_SENSE
# include "../debug.h"
# define ASKDEBUG(a...)  debug_printf("ask_sense: " a)
#else
#defien ASKDEBUG(a...) do { } while(0)
#endif	/* DEBUG_ASK_SENSE */


void
rfm12_ask_sense_start (void)
{
  ASKDEBUG ("initializing.\n");

  /* Initialize Timer0, prescaler 1/256 */
  TCCR0A = 0;
  TCCR0B = _BV(CS02);

  /* Initialize Interrupt */
  _EIMSK |= _BV(RFM12_ASKINT_PIN);
  EICRA = (EICRA & ~RFM12_ASKINT_ISCMASK) | RFM12_ASKINT_ISC;

  last_noise_ts = TCNT0;
  locked_signal = 0;

  rfm12_ask_external_filter_init ();
}


SIGNAL(RFM12_ASKINT_SIGNAL)
{
  uint8_t ts = TCNT0;		/* Get current timestamp. */
  uint8_t delta = DELTA (ts, last_noise_ts);

  if (delta > TIMEOUT_TICKS && locked_signal)
    {
      ASKDEBUG ("timeout.\n");
      locked_signal = 0;
      last_noise_ts = ts;
    }

  if (delta < MIN_TICKS || delta > TIMEOUT_TICKS)
    {
      /* Within noise period, update last_noise_ts and we're done. */
      last_noise_ts = ts;
      return;
    }

  /* We've detected the end of a TX-active phase... */
  if (locked_signal)
    {
      /* ... and it's not the first bit we're receiving. */
    }
  else
    {
      /* ... and this is the first bit ... */
      uint8_t bit = TICKS_TO_BIT (DELTA (ts, last_noise_ts));
      ASKDEBUG ("found start bit=%d", bit);

      locked_signal = 1;
      last_noise_ts = ts;
      last_tx_ts = ts;
    }
}

#endif	/* RFM12_ASK_SENSING_SUPPORT */
