/*
 *         simple rc5 implementation
 *
 *    for additional information please
 *    see http://lochraster.org/etherrape
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "rc5.h"
#include "rc5_net.h"

#define noinline __attribute__((noinline))

/* global variables */
volatile struct rc5_global_t rc5_global;

/* the rc5 bit length (= duration for one bit, encoded in rc5) is 1.8ms
 * (see atmel avr application note AVR410)
 * RC5_BIT_TICKS is the amount of timer ticks for one complete bit
 * RC5_HALF_BIT_TICKS is amount of timer ticks for half a bit
 *
 * (for cpu speed F_CPU and prescaler 1024) */

/* #define RC5_BIT_TICKS ((uint8_t)(F_CPU * 1800 / 1024 / 1000000)) */
#if F_CPU == 20000000UL
#define RC5_BIT_TICKS 35        /* for 20MHz */
#elif F_CPU == 16000000UL
#define RC5_BIT_TICKS 28        /* for 16 MHz */
#else
#define RC5_BIT_TICKS ((uint8_t)(F_CPU * 1800 / 1024 / 1000000))
#endif

#define RC5_HALF_BIT_TICKS ((uint8_t)(RC5_BIT_TICKS/2))

/* define maximal environment for detecting valid rc5 pulses */
#define RC5_ENVIRONMENT ((uint8_t)(RC5_BIT_TICKS / 5))


/*
 * Overview:
 * An rc5 word is composed of 14 bits:  SSTAAAAACCCCCC
 *   - two start bits (always 1)
 *   - one toggle bit (changes value a key is released and pressed again)
 *   - five address bits
 *   - six command bits
 *
 * The signal we actually decode is this 14-bit-word encoded in inverted manchester code.
 *
 *                          +--             --+
 * a one is encoded as:   __|   , inverted:   |__
 *
 *                        --+                 +--
 * a zero is encoded as:    |__ , inverted: __|
 *
 * which means the data signal (already inverted) for the rc5 word
 * 11000010000001 (address 0x02, command 0x01, toggle bit 0) is:
 *
 * bit          S     S     T     A     A     A     A     A     C     C     C     C     C     C
 * value     |  1  |  1  |  0  |  0  |  0  |  0  |  1  |  0  |  0  |  0  |  0  |  0  |  0  |  1  |
 *
 * signal   ----+  +--+     +--+  +--+  +--+  +-----+     +--+  +--+  +--+  +--+  +--+  +-----+  +--
 *              |__|  |_____|  |__|  |__|  |__|     |_____|  |__|  |__|  |__|  |__|  |__|     |__|
 * halfbit-
 * counter         1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
 *
 *
 * Another example: 11100010000010 (address 0x02, command 0x02, toggle bit 1) is:
 *
 * bit          S     S     T     A     A     A     A     A     C     C     C     C     C     C
 * value     |  1  |  1  |  1  |  0  |  0  |  0  |  1  |  0  |  0  |  0  |  0  |  0  |  1  |  0  |
 *
 * signal   ----+  +--+  +--+     +--+  +--+  +-----+     +--+  +--+  +--+  +--+  +-----+     +-----
 *              |__|  |__|  |_____|  |__|  |__|     |_____|  |__|  |__|  |__|  |__|     |_____|
 * halfbit-
 * counter         1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
 *
 *
 * Observation #1: After 27 halfbits we are done in any case. If the last bit
 *                 is a 0, we get the last pin-change interrupt at halfbit 26,
 *                 this is a special case.  In the timeout handling function
 *                 which is called after the last interrupt, we have to check
 *                 if we received 26 or 27 halfbits.  If only 26 halfbits have
 *                 been received, we have to add a zero to the decoded bit-stream.
 *
 * Observation #2: Within a sequence of two zeroes or two ones, an odd number
 *                 of pin-change interrupts occur, and a sequence of a one and
 *                 a zero (the order doesn't matter), contains an even number
 *                 of interrupts:
 *
 *                 |  1  |  1  |        |  0  |  0  |
 *
 *                 ---+  +--+              +--+  +--      3 pin-chages
 *                    |__|  |__         ___|  |__|
 *
 *
 *                 |  1  |  0  |        |  0  |  1  |
 *                                                        2 pin-changes
 *                 ---+     +---           +-----+
 *                    |_____|           ___|     |___
 *
 *
 *                 Conclusion: Every time the bit-sequence alternates between
 *                 zero and one, the parity on the number of pin-change
 *                 interrupts changes.  We can use this to determine, if the
 *                 last transmitted bit was a one or a zero, by just checking
 *                 the parity of the number of interrupts which occured before
 *                 the current one, every second halfbit (which actually means
 *                 we check after each transmitted bit).
 *
 *                 In the beginning, after the halfbit 1 (actually the second,
 *                 but since the first bit is a one, the signal level isn't
 *                 changed and we don't get an interrupt), we know a one has
 *                 transmitted. Therefore if the parity is even, a change in
 *                 the bit-sequence has happenend and the last bit was a zero,
 *                 if the parity is odd another change (or no change) happened,
 *                 the last bit was a one.
 *
 *                 Examples (from above):
 *
 *
 * bit          S     S     T     A     A     A     A     A     C     C     C     C     C     C
 * value     |  1  |  1  |  0  |  0  |  0  |  0  |  1  |  0  |  0  |  0  |  0  |  0  |  0  |  1  |
 *
 * signal   ----+  +--+     +--+  +--+  +--+  +-----+     +--+  +--+  +--+  +--+  +--+  +-----+  +--
 *              |__|  |_____|  |__|  |__|  |__|     |_____|  |__|  |__|  |__|  |__|  |__|     |__|
 * halfbit-
 * counter      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
 *
 * interrupt-   0  1  2  3  3  4  5  6  7  8  9  10 10 11 11 12 13 14 15 16 17 18 19 20 21 22 22 23
 * counter         |     |     |     |     |     |     |     |     |     |     |     |     |     |
 *                 v     v     v     v     v     v     v     v     v     v     v     v     v     v
 *
 * received        1     1     0     0     0     0     1     0     0     0     0     0     0     1
 * bit
 *
 *
 * Another example: 11100010000010 (address 0x02, command 0x02, toggle bit 1) is:
 *
 * bit          S     S     T     A     A     A     A     A     C     C     C     C     C     C
 * value     |  1  |  1  |  1  |  0  |  0  |  0  |  1  |  0  |  0  |  0  |  0  |  0  |  1  |  0  |
 *
 * signal   ----+  +--+  +--+     +--+  +--+  +-----+     +--+  +--+  +--+  +--+  +-----+     +-----
 *              |__|  |__|  |_____|  |__|  |__|     |_____|  |__|  |__|  |__|  |__|     |_____|
 * halfbit-
 * counter      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
 *
 * interrupt-   0  1  2  3  4  5  5  6  7  8  9  10 10 11 11 12 13 14 15 16 17 18 19 20 20 21 22 24
 * counter         |     |     |     |     |     |     |     |     |     |     |     |     |     |
 *                 v     v     v     v     v     v     v     v     v     v     v     v     v     v
 *
 * received        1     1     1     0     0     0     1     0     0     0     0     0     1     0
 * bit
 *
 *
 *
 *
 * General Roadmap: 1) Interrupt on the first falling edge, activate and reset a
 *                     timer, reset halfbit-counter and interrupt-counter, switch
 *                     interrupt to fire at any logical change, enable timer
 *                     overflow interrupt.
 *                  2) In external interrupt, check if one or two halfbits have
 *                     passed between this and the last interrupt by checking the
 *                     timer (using RC5_BIT_TICKS/RC5_HALF_TICKS and RC5_ENVIRONMENT),
 *                     process each detected halfbit, increment the
 *                     interrupt-counter afterwards.
 *                  3) In timer overflow interrupt (timeout), check if 26 (add a zero)
 *                     or 27 halfbits have been received, copy output, reconfigure
 *                     interrupt to fire at the falling edge again, disable timer
 *                     overflow interrupt, copy output to global public structure.
 *
 */

/* local variables, temporary command buffer */
static struct rc5_t temp_rc5;

/* module local prototypes */
static void noinline rc5_send_one(void);
static void noinline rc5_send_zero(void);

void
rc5_init(void)
{

  /* configure send pin as output, set low */
  DDR_CONFIG_OUT(RC5_SEND);
  PIN_CLEAR(RC5_SEND);

  /* enable timer0/timer2, set prescaler and enable overflow interrupt */
#ifdef RC5_USE_TIMER2
  TC2_PRESCALER_1024;
#else
  TC0_PRESCALER_1024;
#endif

  /* configure int0 to fire at any logical change */
  EICRA |= _BV(RC5_ISC0);
  EICRA &= ~_BV(RC5_ISC1);

  /* clear any old interrupts and enable int0 interrupt */
  EIFR = _BV(RC5_INT_PIN);
  EIMSK |= _BV(RC5_INT_PIN);

  /* reset everything to zero */
  memset((void *) &rc5_global, 0, sizeof(rc5_global));

  /* enable rc5 receive, init variables */
  rc5_global.enabled = 1;
}

void
rc5_send(const uint8_t addr, const uint8_t cmd)
{
  static uint8_t toggle = 0;

  /* send two one sync bits */
  rc5_send_one();
  rc5_send_one();

  /* send toggle bit */
  if (toggle)
    rc5_send_one();
  else
    rc5_send_zero();

  toggle = !toggle;

  for (int8_t i = 4; i >= 0; i--)
  {
    if (addr & _BV(i))
      rc5_send_one();
    else
      rc5_send_zero();
  }

  for (int8_t i = 5; i >= 0; i--)
  {
    if (cmd & _BV(i))
      rc5_send_one();
    else
      rc5_send_zero();
  }

  /* turn off sender */
  PIN_CLEAR(RC5_SEND);
}

void
rc5_send_one(void)
{
  PIN_CLEAR(RC5_SEND);
  _delay_loop_2(RC5_PULSE);
  PIN_SET(RC5_SEND);
  _delay_loop_2(RC5_PULSE);
}

void
rc5_send_zero(void)
{
  PIN_SET(RC5_SEND);
  _delay_loop_2(RC5_PULSE);
  PIN_CLEAR(RC5_SEND);
  _delay_loop_2(RC5_PULSE);
}

void
rc5_process(void)
{
  static uint8_t toggle = 2;

  if (rc5_global.new_data)
  {
#ifdef DEBUG_RC5
    debug_printf("received new rc5 data: addr: %d, cmd %d, toggle %d\n",
                 rc5_global.received_command.address,
                 rc5_global.received_command.code,
                 rc5_global.received_command.toggle_bit);

#ifdef RC5_UDP_SUPPORT_COUNTERS
    char s[200];
    for (uint8_t i = 0; i < rc5_global.bitcount; i++)
    {
      uint8_t saddr = i * 3;
      snprintf(&s[saddr], sizeof(s) - saddr, "%02x ", rc5_global.cnt[i]);
    }
    debug_printf("counters: %d [%s]\n", rc5_global.bitcount, s);
#endif

#endif
    if (toggle != rc5_global.received_command.toggle_bit)
    {

#ifdef DEBUG_RC5
      debug_printf("new keypress, queue len is %d:\n", rc5_global.len);
      for (uint8_t i = 0; i < rc5_global.len; i++)
        debug_printf("  addr %d, cmd %d, toggle bit %d\n",
                     rc5_global.queue[i].address,
                     rc5_global.queue[i].code,
                     rc5_global.queue[i].toggle_bit);

#endif

      /* shift queue backwards */
      memmove((char *) &rc5_global.queue[1],
              (char *) &rc5_global.queue[0],
              (RC5_QUEUE_LENGTH - 1) * sizeof(struct rc5_t));

      /* copy datagram to queue and increment length */
      rc5_global.queue[0].raw = rc5_global.received_command.raw;
      if (rc5_global.len < RC5_QUEUE_LENGTH)
        rc5_global.len++;

      toggle = rc5_global.received_command.toggle_bit;

    }

#ifdef RC5_UDP_SUPPORT
    rc5_udp_send();             /* send UDP packet */
#endif
#ifdef RC5_UDP_SUPPORT_COUNTERS
    rc5_global.bitcount = 0;    /* delete packet */
#endif

    rc5_global.new_data = 0;
  }
}

ISR(RC5_INT_VECTOR)
{
  if (rc5_global.enabled && !rc5_global.temp_disable)
  {

    /* if this is the first interrupt */
    if (rc5_global.interrupts == 0)
    {
      /* reset counter, clear old overflows and enable
       * timer0 overflow interrupt */
#ifdef RC5_USE_TIMER2
      TC2_COUNTER_CURRENT = 0;
      TC2_INT_OVERFLOW_CLR;
      TC2_INT_OVERFLOW_ON;
#else
      TC0_COUNTER_CURRENT = 0;
      TC0_INT_OVERFLOW_CLR;
      TC0_INT_OVERFLOW_ON;
#endif

      /* reset temp buffer */
      temp_rc5.raw = 0;
#ifdef RC5_UDP_SUPPORT_COUNTERS
      /* reset buffer */
      temp_rc5.bitcount = 0;

      /* set buffer start byte */
      temp_rc5.cnt[temp_rc5.bitcount++] = 0xff;
#endif
      /* if this is not the first interrupt */
    }
    else
    {
      /* load and reset the counter */
#ifdef RC5_USE_TIMER2
      uint8_t counter = TC2_COUNTER_CURRENT;
      TC2_COUNTER_CURRENT = 0;
#else
      uint8_t counter = TC0_COUNTER_CURRENT;
      TC0_COUNTER_CURRENT = 0;
#endif

      /* check how many halfbits have passed since last interrupt */
      uint8_t received_bits = 0;

      /* check for two halfbits */
      if ((counter > RC5_HALF_BIT_TICKS - RC5_ENVIRONMENT &&
           counter < RC5_HALF_BIT_TICKS + RC5_ENVIRONMENT))
      {
        received_bits = 1;

        /* check for one halfbit */
      }
      else if ((counter > RC5_BIT_TICKS - RC5_ENVIRONMENT &&
                counter < RC5_BIT_TICKS + RC5_ENVIRONMENT))
      {
        received_bits = 2;

        /* else signal is invalid */
      }
      else
      {
        /* disable interrupt, enable timer overflow interrupt: -> timeout */
        //GICR &= ~_BV(INT0);
        rc5_global.temp_disable = 1;

        /* reset bitcounter */
        rc5_global.halfbitcount = 0;
#ifdef RC5_UDP_SUPPORT_COUNTERS
        temp_rc5.bitcount = 0;
#endif
        /* quit this interrupt and wait for timeout */
        return;
      }

#ifdef RC5_UDP_SUPPORT_COUNTERS
      /* add a counter value to the counter buffer */
      temp_rc5.cnt[temp_rc5.bitcount++] = counter;
#endif
      /* process each received halfbit */
      while (received_bits--)
      {

        /* increase halfbit counter, since we obviously
         * received a halfbit */
        rc5_global.halfbitcount++;

        /* if the parity of the halfbit-counter is odd,
         * we have received a complete bit,
         * so save this bit to the structure*/

        if (rc5_global.halfbitcount & 1)
        {
          /* shift buffer left, and set lsb, if the
           * parity of the interrupt counter (interrupts
           * which happened _BEFORE_ this one (=postincrement))
           * is odd*/
          temp_rc5.raw <<= 1;
          temp_rc5.raw |= (rc5_global.interrupts & 1);
        }
      }
    }

    /* increase interrupt counter */
    rc5_global.interrupts++;
  }
}

/* timer0 overflow interrupt */
#ifdef RC5_USE_TIMER2
ISR(TC2_VECTOR_OVERFLOW)
#else
ISR(TC0_VECTOR_OVERFLOW)
#endif
{
#ifdef RC5_UDP_SUPPORT_COUNTERS
  /* disable overflow interrupt */
#ifdef RC5_USE_TIMER2
  TC2_INT_OVERFLOW_OFF;
#else
  TC0_INT_OVERFLOW_OFF;
#endif

#ifdef RC5_USE_TIMER2
  uint8_t counter = TC2_COUNTER_CURRENT;
#else
  uint8_t counter = TC0_COUNTER_CURRENT;
#endif
#endif /* RC5_UDP_SUPPORT_COUNTERS */

  /* check if we only received 26 halfbits,
   * so the last transmitted bit was zero,
   * if 27 halfbits have been received, everything
   * went fine */
  switch (rc5_global.halfbitcount)
  {
    case 26:
      /* add a zero */
      temp_rc5.raw <<= 1;
      /* and fall-through to the next case */

    case 27:
      /* copy data to global structure */
      rc5_global.received_command.raw = temp_rc5.raw;

#ifdef RC5_UDP_SUPPORT_COUNTERS
      /* add buffer end */
      temp_rc5.cnt[temp_rc5.bitcount++] = 0xFF;

      /* copy temp buffer to global buffer space */
      rc5_global.bitcount = temp_rc5.bitcount;
      memcpy(&rc5_global.cnt, temp_rc5.cnt, RC5_COUNTERS);
#endif
      /* signal main that new data has arrived */
      rc5_global.new_data = 1;
  }

  /* reset counter */
  rc5_global.halfbitcount = 0;
  rc5_global.interrupts = 0;

#ifdef RC5_UDP_SUPPORT_COUNTERS
  /* clean up counter buffer */
  temp_rc5.bitcount = 0;
  memset((void *) temp_rc5.cnt, 0, RC5_COUNTERS);
#endif
  /* if decoder is enabled, reconfigure int0 */
  if (rc5_global.enabled)
  {

    /* re-enable int0 */
    rc5_global.temp_disable = 0;
  }
}

/*
  -- Ethersex META --
  header(hardware/ir/rc5/rc5.h)
  mainloop(rc5_process)
  init(rc5_init)
*/
