/* vim: fdm=marker ts=4 et ai
 * {{{
 *
 *          fs20 sender implementation
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <avr/io.h>
#include <util/delay.h>
#include <util/parity.h>
#include <avr/interrupt.h>
#include <string.h>
#include "fs20.h"
#include "bit-macros.h"
#include "debug.h"

#ifdef FS20_SUPPORT

/* module-local prototypes */
#ifdef FS20_SUPPORT_SEND
/* prototypes for sending fs20 */
static void fs20_send_zero(void);
static void fs20_send_one(void);
static void fs20_send_sync(void);
static inline void fs20_send_bit(uint8_t bit);
static inline void fs20_send_byte(uint8_t byte);
#endif /* FS20_SUPPORT_SEND */

#ifdef FS20_SUPPORT_RECEIVE
/* prototypes for receiving fs20 */

#endif /* FS20_SUPPORT_RECEIVE */


/* global variables */
volatile struct fs20_global_t fs20_global;


#ifdef FS20_SUPPORT_SEND

void fs20_send_zero(void)
/* {{{ */ {

    FS20_SEND_PORT |= _BV(FS20_SEND_PINNUM);
    _delay_loop_2(FS20_DELAY_ZERO);
    FS20_SEND_PORT &= ~_BV(FS20_SEND_PINNUM);
    _delay_loop_2(FS20_DELAY_ZERO);

} /* }}} */

void fs20_send_one(void)
/* {{{ */ {

    FS20_SEND_PORT |= _BV(FS20_SEND_PINNUM);
    _delay_loop_2(FS20_DELAY_ONE);
    FS20_SEND_PORT &= ~_BV(FS20_SEND_PINNUM);
    _delay_loop_2(FS20_DELAY_ONE);

} /* }}} */

void fs20_send_sync(void)
/* {{{ */ {

    for (uint8_t i = 0; i < 12; i++)
        fs20_send_zero();

    fs20_send_one();

} /* }}} */

void fs20_send_bit(uint8_t bit)
/* {{{ */ {

    if (bit > 0)
        fs20_send_one();
    else
        fs20_send_zero();

} /* }}} */

void fs20_send_byte(uint8_t byte)
/* {{{ */ {

    uint8_t i = 7;

    do {
        fs20_send_bit(byte & _BV(i));
    } while (i-- > 0);

    fs20_send_bit(parity_even_bit(byte));

} /* }}} */

void fs20_send(uint16_t housecode, uint8_t address, uint8_t command)
/* {{{ */ {

    for (uint8_t i = 0; i < 3; i++) {
        fs20_send_sync();

        uint8_t sum = 6; /* magic constant 6 from fs20 protocol definition... */

        fs20_send_byte(HI8(housecode));
        sum += HI8(housecode);
        fs20_send_byte(LO8(housecode));
        sum += LO8(housecode);
        fs20_send_byte(address);
        sum += address;
        fs20_send_byte(command);
        sum += command;
        fs20_send_byte(sum);

        fs20_send_zero();

        _delay_loop_2(FS20_DELAY_CMD);
    }

} /* }}} */

#endif /* FS20_SUPPORT_SEND */

#ifdef FS20_SUPPORT_RECEIVE

ISR(ANALOG_COMP_vect)
/* {{{ */ {
    /* if locked or timeout > 0, return */
    if (fs20_global.timeout > 0 ||
        fs20_global.rec == FS20_DATAGRAM_LENGTH)
        return;

    /* save counter for processing, reset counter */
    static uint8_t time_old = 0;
    uint8_t time = TCNT2;
    TCNT2 = 0;

    /* check value */
    if (FS20_PULSE_ZERO(time) &&
        FS20_PULSE_ZERO(time_old) &&
        FS20_PULSE_DIFFERENCE(time, time_old)) {

        /* we received a zero */
        time_old = 0;
        fs20_global.err = 0;
        fs20_global.raw <<= 1;
        fs20_global.rec++;
    } else if (FS20_PULSE_ONE(time) &&
               FS20_PULSE_ONE(time_old) &&
               FS20_PULSE_DIFFERENCE(time, time_old)) {

        /* we received a one */
        time_old = 0;
        fs20_global.err = 0;
        fs20_global.raw <<= 1;
        fs20_global.raw |= 1;
        fs20_global.rec++;
    } else {
        if (fs20_global.err > 3) {
            fs20_global.err = 0;
            fs20_global.rec = 0;
            time_old = 0;
            fs20_global.raw = 0;
        } else {
            time_old = time;
            fs20_global.err++;
        }
    }
} /* }}} */

ISR(TIMER2_OVF_vect)
/* {{{ */ {
    /* reset data structures, if not locked */
    if (fs20_global.rec != FS20_DATAGRAM_LENGTH ||
        fs20_global.timeout > 0) {
        fs20_global.rec = 0;
        fs20_global.raw = 0;
    }
} /* }}} */

void fs20_process(void)
/* {{{ */ {

    /* check if something has been received */
    if (fs20_global.rec == 58) {
#ifdef DEBUG_FS20_REC
        debug_printf("received new fs20 datagram, queue fill is %u:\n", fs20_global.len);

        for (uint8_t l = 0; l < fs20_global.len; l++) {
            struct fs20_datagram_t *dg = &fs20_global.queue[l];

            debug_printf("%u: %02x%02x addr %02x cmd %02x\n", l,
                    dg->hc1, dg->hc2,
                    dg->addr, dg->cmd);
        }

#endif

        if (fs20_global.datagram.sync == 0x0001) {
#ifdef DEBUG_FS20_REC
            debug_printf("valid sync\n");
#endif

            /* create shortcut to fs20_global.datagram */
            volatile struct fs20_datagram_t *dg = &fs20_global.datagram;

            /* check parity */
            uint8_t p1, p2, p3, p4, p5;
            uint8_t parity = 6; /* magic constant from fs20 protocol definition */

            p1 = parity_even_bit(dg->hc1)    ^ dg->p1;
            p2 = parity_even_bit(dg->hc2)    ^ dg->p2;
            p3 = parity_even_bit(dg->addr)   ^ dg->p3;
            p4 = parity_even_bit(dg->cmd)    ^ dg->p4;
            p5 = parity_even_bit(dg->parity) ^ dg->p5;

            parity += dg->hc1
                    + dg->hc2
                    + dg->addr
                    + dg->cmd;

            /* check parity */
            if (!p1 && !p2 && !p3 && !p4 && !p5 && parity == dg->parity) {
#ifdef DEBUG_FS20_REC
                debug_printf("valid datagram\n");
#endif
                /* shift queue backwards */
#ifdef DEBUG_FS20_REC
                debug_printf("moving queue around...\n");
#endif
                memmove(&fs20_global.queue[1],
                        &fs20_global.queue[0],
                        (FS20_QUEUE_LENGTH-1) * sizeof(struct fs20_datagram_t));

                /* copy datagram to queue */
                memcpy(&fs20_global.queue[0],
                        (const void *)&fs20_global.datagram,
                        sizeof(struct fs20_datagram_t));

                if (fs20_global.len < FS20_QUEUE_LENGTH)
                    fs20_global.len++;

                /* set timeout (for 120ms = 6 * 20ms), if received a complete packet */
                fs20_global.timeout = 6;
            } else {
#ifdef DEBUG_FS20_REC
                debug_printf("invalid datagram\n");
#endif
            }
        } else {
#ifdef DEBUG_FS20_REC
            debug_printf("sync invalid!\n");
#endif
        }

        fs20_global.raw = 0;
        fs20_global.rec = 0;
    }

} /* }}} */

void fs20_process_timeout(void)
/* {{{ */ {

    /* clear fs20 timeout */
    if (fs20_global.timeout > 0)
        fs20_global.timeout--;

} /* }}} */

#endif /* FS20_SUPPORT_RECEIVE */


void fs20_init(void)
/* {{{ */ {
    /* default: enabled */
    fs20_global.enable = 1;

#ifdef FS20_SUPPORT_SEND
    /* configure port pin for sending */
    FS20_SEND_DDR |= _BV(FS20_SEND_PINNUM);
    FS20_SEND_PORT &= ~_BV(FS20_SEND_PINNUM);
#endif

#ifdef FS20_SUPPORT_RECEIVE
    /* reset global data structures */
    fs20_global.raw = 0;
    fs20_global.err = 0;
    fs20_global.rec = 0;
    fs20_global.len = 0;
    fs20_global.timeout = 0;

    /* configure port pin for use as input to the analoge comparator */
    FS20_SEND_DDR &= ~_BV(FS20_SEND_PINNUM);
    FS20_SEND_PORT &= ~_BV(FS20_SEND_PINNUM);

    /* enable analog comparator,
     * use fixed voltage reference (1V, connected to AIN0)
     * reset interrupt flag (ACI)
     * enable interrupt
     */
    ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIE);

    /* configure timer2 for receiving fs20,
     * prescaler 128
     * overflow interrupt enabled */
    TCNT2 = 0;
    TCCR2A = 0;
    TCCR2B = _BV(CS20) | _BV(CS22);
    TIMSK2 = _BV(TOIE2);
#endif
} /* }}} */

#endif
