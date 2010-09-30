/*
 *         simple irmp implementation
 *
 *    for additional information please
 *    see http://www.mikrocontroller.net/articles/IRMP
 *
 * (c) by eku
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
#include "irmp.h"

#define noinline __attribute__((noinline))

/* global variables */
//volatile struct irmp_global_t irmp_global;

/* the irmp bit length (= duration for one bit, encoded in irmp) is 1.8ms
 * (see atmel avr application note AVR410)
 * IRMP_BIT_TICKS is the amount of timer ticks for one complete bit
 * IRMP_HALF_BIT_TICKS is amount of timer ticks for half a bit
 *
 * (for cpu speed F_CPU and prescaler 1024) */

/* #define IRMP_BIT_TICKS ((uint8_t)(F_CPU * 1800 / 1024 / 1000000)) */
#if F_CPU == 20000000UL
#define IRMP_BIT_TICKS 35    /* for 20MHz */
#elif F_CPU == 16000000UL
#define IRMP_BIT_TICKS 28    /* for 16 MHz */
#else
#define IRMP_BIT_TICKS ((uint8_t)(F_CPU * 1800 / 1024 / 1000000)) 
#endif

#define IRMP_HALF_BIT_TICKS ((uint8_t)(IRMP_BIT_TICKS/2))

/* define maximal environment for detecting valid irmp pulses */
#define IRMP_ENVIRONMENT ((uint8_t)(IRMP_BIT_TICKS / 5))


/* local variables, temporary command buffer */
//static struct irmp_t temp_irmp;

/* module local prototypes */

void irmp_init(void)
{
#if 0

    /* configure send pin as output, set low */
    DDR_CONFIG_OUT(IRMP_SEND);
    PIN_CLEAR(IRMP_SEND);

    /* enable timer0/timer2, set prescaler and enable overflow interrupt */
#ifdef IRMP_USE_TIMER2
    TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20);
#else
    TCCR0A = 0;
    TCCR0B = _BV(CS02) | _BV(CS00);
#endif

    /* configure int0 to fire at any logical change */
    EICRA |= _BV(IRMP_ISC0);
    EICRA &= ~_BV(IRMP_ISC1);

    /* clear any old interrupts and enable int0 interrupt */
    EIFR = _BV(IRMP_INT_PIN);
    EIMSK |= _BV(IRMP_INT_PIN);

    /* reset everything to zero */
    memset((void *)&irmp_global, 0, sizeof(irmp_global));

    /* enable irmp receive, init variables */
    irmp_global.enabled = 1;
#endif
}

void irmp_send(uint8_t addr, uint8_t cmd)
{
#if 0
    static uint8_t toggle = 0;

    /* send two one sync bits */
    irmp_send_one();
    irmp_send_one();

    /* send toggle bit */
    if (toggle)
        irmp_send_one();
    else
        irmp_send_zero();

    toggle = !toggle;

    for (int8_t i = 4; i >= 0; i--) {
        if (addr & _BV(i))
            irmp_send_one();
        else
            irmp_send_zero();
    }

    for (int8_t i = 5; i >= 0; i--) {
        if (cmd & _BV(i))
            irmp_send_one();
        else
            irmp_send_zero();
    }

    /* turn off sender */
    PIN_CLEAR(IRMP_SEND);
#endif
}

void irmp_process(void)
{
#if 0
    static uint8_t toggle = 2;

    if (irmp_global.new_data) {
#ifdef DEBUG_IRMP
        debug_printf("received new irmp data: addr: %d, cmd %d, toggle %d\n",
                irmp_global.received_command.address,
                irmp_global.received_command.code,
                irmp_global.received_command.toggle_bit);

#ifdef IRMP_UDP_SUPPORT_COUNTERS
	char s[200];
        for (uint8_t i = 0; i < irmp_global.bitcount; i++) {
            uint8_t saddr = i*3;
            snprintf(&s[saddr], sizeof(s) - saddr, "%02x ", irmp_global.cnt[i]);
        }
        debug_printf("counters: %d [%s]\n", irmp_global.bitcount, s);
#endif

#endif
        if (toggle != irmp_global.received_command.toggle_bit) {

#ifdef DEBUG_IRMP
            debug_printf("new keypress, queue len is %d:\n", irmp_global.len);
            for (uint8_t i = 0; i < irmp_global.len; i++)
                debug_printf("  addr %d, cmd %d, toggle bit %d\n",
                        irmp_global.queue[i].address,
                        irmp_global.queue[i].code,
                        irmp_global.queue[i].toggle_bit);

#endif

            /* shift queue backwards */
            memmove((char *) &irmp_global.queue[1],
                    (char *) &irmp_global.queue[0],
                    (IRMP_QUEUE_LENGTH-1) * sizeof(struct irmp_t));

            /* copy datagram to queue and increment length */
            irmp_global.queue[0].raw = irmp_global.received_command.raw;
            if (irmp_global.len < IRMP_QUEUE_LENGTH) 
                irmp_global.len++;

            toggle = irmp_global.received_command.toggle_bit;

        }

#ifdef IRMP_UDP_SUPPORT
        irmp_udp_send();          /* send UDP packet */
#endif
#ifdef IRMP_UDP_SUPPORT_COUNTERS
        irmp_global.bitcount = 0; /* delete packet */
#endif
        
        irmp_global.new_data = 0;
    }
}

ISR(IRMP_INT_SIGNAL)
{

    if (irmp_global.enabled && !irmp_global.temp_disable) {

        /* if this is the first interrupt */
        if (irmp_global.interrupts == 0) {
            /* reset counter, clear old overflows and enable
             * timer0 overflow interrupt */
#ifdef IRMP_USE_TIMER2
            TCNT2 = 0;
            _TIFR_TIMER2 = _BV(TOV2);
            _TIMSK_TIMER2 |= _BV(TOIE2);
#else
            TCNT0 = 0;
            TIFR0 = _BV(TOV0);
            TIMSK0 |= _BV(TOIE0);
#endif

            /* reset temp buffer */
            temp_irmp.raw = 0;
#ifdef IRMP_UDP_SUPPORT_COUNTERS
            /* reset buffer */
            temp_irmp.bitcount = 0;

            /* set buffer start byte */
            temp_irmp.cnt[temp_irmp.bitcount++] = 0xff;
#endif
            /* if this is not the first interrupt */
        } else {
            /* load and reset the counter */
#ifdef IRMP_USE_TIMER2
            uint8_t counter = TCNT2;
            TCNT2 = 0;
#else
            uint8_t counter = TCNT0;
            TCNT0 = 0;
#endif

            /* check how many halfbits have passed since last interrupt */
            uint8_t received_bits = 0;

            /* check for two halfbits */
            if ( (counter > IRMP_HALF_BIT_TICKS - IRMP_ENVIRONMENT &&
                  counter < IRMP_HALF_BIT_TICKS + IRMP_ENVIRONMENT) ) {
                received_bits = 1;

            /* check for one halfbit */
            } else if ( (counter > IRMP_BIT_TICKS - IRMP_ENVIRONMENT &&
                         counter < IRMP_BIT_TICKS + IRMP_ENVIRONMENT) ) {
                received_bits = 2;

            /* else signal is invalid */
            } else {
                /* disable interrupt, enable timer overflow interrupt: -> timeout */
                //GICR &= ~_BV(INT0);
                irmp_global.temp_disable = 1;

                /* reset bitcounter */
                irmp_global.halfbitcount = 0;
#ifdef IRMP_UDP_SUPPORT_COUNTERS
                temp_irmp.bitcount = 0;
#endif
                /* quit this interrupt and wait for timeout */
                return;
            }

#ifdef IRMP_UDP_SUPPORT_COUNTERS
            /* add a counter value to the counter buffer */
            temp_irmp.cnt[temp_irmp.bitcount++] = counter;
#endif
            /* process each received halfbit */
            while (received_bits--) {

                /* increase halfbit counter, since we obviously
                 * received a halfbit */
                irmp_global.halfbitcount++;

                /* if the parity of the halfbit-counter is odd,
                 * we have received a complete bit,
                 * so save this bit to the structure*/

                if (irmp_global.halfbitcount & 1) {
                    /* shift buffer left, and set lsb, if the
                     * parity of the interrupt counter (interrupts
                     * which happened _BEFORE_ this one (=postincrement))
                     * is odd*/
                    temp_irmp.raw <<= 1;
                    temp_irmp.raw |= (irmp_global.interrupts & 1);
                }
            }
        }

        /* increase interrupt counter */
        irmp_global.interrupts++;
    }
#endif
}

/* timer0 overflow interrupt */
#ifdef IRMP_USE_TIMER2
ISR(TIMER2_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
#if 0

#ifdef IRMP_UDP_SUPPORT_COUNTERS
    /* disable overflow interrupt */
#ifdef IRMP_USE_TIMER2
    _TIMSK_TIMER2 &= ~_BV(TOIE2);
#else
    TIMSK0 &= ~_BV(TOIE0);
#endif

#ifdef IRMP_USE_TIMER2
            uint8_t counter = TCNT2;
#else
            uint8_t counter = TCNT0;
#endif
#endif /* IRMP_UDP_SUPPORT_COUNTERS */

    /* check if we only received 26 halfbits,
     * so the last transmitted bit was zero,
     * if 27 halfbits have been received, everything
     * went fine */
    switch (irmp_global.halfbitcount) {
        case 26:
                 /* add a zero */
                 temp_irmp.raw <<= 1;
                 /* and fall-through to the next case */

        case 27:
                 /* copy data to global structure */
                 irmp_global.received_command.raw = temp_irmp.raw;

#ifdef IRMP_UDP_SUPPORT_COUNTERS
                 /* add buffer end */
                 temp_irmp.cnt[temp_irmp.bitcount++] = 0xFF;

                 /* copy temp buffer to global buffer space */
                 irmp_global.bitcount = temp_irmp.bitcount;
                 memcpy(&irmp_global.cnt, temp_irmp.cnt, IRMP_COUNTERS);
#endif
                 /* signal main that new data has arrived */
                 irmp_global.new_data = 1;
    }

    /* reset counter */
    irmp_global.halfbitcount = 0;
    irmp_global.interrupts = 0;

#ifdef IRMP_UDP_SUPPORT_COUNTERS
    /* clean up counter buffer */
    temp_irmp.bitcount = 0;
    memset((void *) temp_irmp.cnt, 0, IRMP_COUNTERS);
#endif
    /* if decoder is enabled, reconfigure int0 */
    if (irmp_global.enabled) {

        /* re-enable int0 */
        irmp_global.temp_disable = 0;
    }
#endif
}

/*
  -- Ethersex META --
  header(hardware/ir/irmp/irmp.h)
  mainloop(irmp_process)
  init(irmp_init)
*/
