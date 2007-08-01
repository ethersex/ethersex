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
#include "../bit-macros.h"
#include "../debug.h"

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
#ifdef FS20_SUPPORT_RECEIVE_WS300
void ws300_parse_datagram(void);
#endif

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
#ifdef FS20_RECV_PROFILE
    fs20_global.int_counter++;
#endif

    /* save counter for later processing and reset timer */
    uint8_t time = TCNT2;
    TCNT2 = 0;

    /* if fs20 locked or timeout > 0, continue */
    if (fs20_global.fs20.timeout == 0 &&
        fs20_global.fs20.rec < FS20_DATAGRAM_LENGTH) {

        static uint8_t time_old = 0;

        /* check value */
        if (FS20_PULSE_ZERO(time) &&
                FS20_PULSE_ZERO(time_old) &&
                FS20_PULSE_DIFFERENCE(time, time_old)) {

            /* we received a zero */
            time_old = 0;
            fs20_global.fs20.err = 0;
            fs20_global.fs20.raw <<= 1;
            fs20_global.fs20.rec++;
        } else if (FS20_PULSE_ONE(time) &&
                FS20_PULSE_ONE(time_old) &&
                FS20_PULSE_DIFFERENCE(time, time_old)) {

            /* we received a one */
            time_old = 0;
            fs20_global.fs20.err = 0;
            fs20_global.fs20.raw <<= 1;
            fs20_global.fs20.raw |= 1;
            fs20_global.fs20.rec++;
        } else {
            if (fs20_global.fs20.err > 3) {
                fs20_global.fs20.err = 0;
                fs20_global.fs20.rec = 0;
                time_old = 0;
                fs20_global.fs20.raw = 0;
            } else {
                time_old = time;
                fs20_global.fs20.err++;
            }
        }
    }

#ifdef FS20_SUPPORT_RECEIVE_WS300
    /* if ws300 is not locked, continue */
    if (fs20_global.ws300.rec < FS20_WS300_DATAGRAM_LENGTH) {

        /* save counter for processing */
        static uint8_t time_old = 0;

        int8_t v;

        if (WS300_PULSE_ZERO(time_old, time))
            v = 0;
        else if (WS300_PULSE_ONE(time_old, time))
            v = 1;
        else
            v = -1;

        /* if we aren't synced */
        if (!fs20_global.ws300.sync) {
            if (v == 0) {
                fs20_global.ws300.null++;
                time_old = 0;
            } else if (v == 1) {
                if (fs20_global.ws300.null >= 8) {
                    fs20_global.ws300.sync = 1;
                    time_old = 0;
                } else {
                    time_old = time;
                }

                fs20_global.ws300.null = 0;
            } else {
                time_old = time;
            }
        } else {
            /* else we are synced */
            if (v >= 0) {

                // printf("%u", v);
                uint8_t byte = fs20_global.ws300.rec / 8;
                uint8_t bit = fs20_global.ws300.rec % 8;

                if (v == 1)
                    fs20_global.ws300.bytes[byte] |= _BV(bit);

                time_old = 0;
                fs20_global.ws300.rec++;
            } else
                time_old = time;
        }
    }
#endif
} /* }}} */

ISR(TIMER2_OVF_vect)
/* {{{ */ {
#ifdef FS20_RECV_PROFILE
    fs20_global.ovf_counter++;
#endif

    /* reset data structures, if not locked */
    if (fs20_global.fs20.rec != FS20_DATAGRAM_LENGTH ||
        fs20_global.fs20.timeout > 0) {
        fs20_global.fs20.rec = 0;
        memset((void *)&fs20_global.fs20.datagram, 0, sizeof(struct fs20_datagram_t));
    }

#ifdef FS20_SUPPORT_RECEIVE_WS300
    if (fs20_global.ws300.rec != FS20_WS300_DATAGRAM_LENGTH) {
        fs20_global.ws300.rec = 0;
        fs20_global.ws300.sync = 0;
        fs20_global.ws300.null = 0;
        memset((void *)&fs20_global.ws300.datagram, 0, sizeof(struct ws300_datagram_t));
    }
#endif

} /* }}} */

void fs20_process(void)
/* {{{ */ {

    /* check if something has been received */
    if (fs20_global.fs20.rec == 58) {
#ifdef DEBUG_FS20_REC
        debug_printf("received new fs20 datagram, queue fill is %u:\n", fs20_global.fs20.len);

        for (uint8_t l = 0; l < fs20_global.fs20.len; l++) {
            struct fs20_datagram_t *dg = &fs20_global.fs20.queue[l];

            debug_printf("%u: %02x%02x addr %02x cmd %02x\n", l,
                    dg->hc1, dg->hc2,
                    dg->addr, dg->cmd);
        }

#endif

        if (fs20_global.fs20.datagram.sync == 0x0001) {
#ifdef DEBUG_FS20_REC
            debug_printf("valid sync\n");
#endif

            /* create shortcut to fs20_global.datagram */
            volatile struct fs20_datagram_t *dg = &fs20_global.fs20.datagram;

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
                memmove(&fs20_global.fs20.queue[1],
                        &fs20_global.fs20.queue[0],
                        (FS20_QUEUE_LENGTH-1) * sizeof(struct fs20_datagram_t));

                /* copy datagram to queue */
                memcpy(&fs20_global.fs20.queue[0],
                        (const void *)&fs20_global.fs20.datagram,
                        sizeof(struct fs20_datagram_t));

                if (fs20_global.fs20.len < FS20_QUEUE_LENGTH)
                    fs20_global.fs20.len++;

                /* set timeout (for 120ms = 6 * 20ms), if received a complete packet */
                fs20_global.fs20.timeout = 6;
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

        fs20_global.fs20.raw = 0;
        fs20_global.fs20.rec = 0;
    }

#ifdef FS20_SUPPORT_RECEIVE_WS300
    if (fs20_global.ws300.rec == FS20_WS300_DATAGRAM_LENGTH) {
        #ifdef DEBUG_FS20_WS300
        debug_printf("received ws300 datagram\n");
        #endif

        #ifdef DEBUG_FS20_WS300_VERBOSE
        for (uint8_t i = 0; i < 10; i++)
            for (uint8_t j = 0; j < 8; j++) {

                if ( (i*8+j) % 5 == 4 )
                    printf(" ");

                printf("%u", (fs20_global.ws300.bytes[i] & _BV(j)) > 0);

                if ( (i*8+j) % 5 == 4 )
                    printf(" ");

            }

        printf("\n");
        #endif

        ws300_parse_datagram();

        /* clear global data structure */
        memset((void *)&fs20_global.ws300.datagram, 0, sizeof(struct ws300_datagram_t));
        fs20_global.ws300.sync = 0;
        fs20_global.ws300.rec = 0;
    }
#endif
} /* }}} */

void fs20_process_timeout(void)
/* {{{ */ {

    /* clear fs20 timeout */
    if (fs20_global.fs20.timeout > 0)
        fs20_global.fs20.timeout--;

} /* }}} */

#ifdef FS20_SUPPORT_RECEIVE_WS300
void ws300_parse_datagram(void)
/* {{{ */ {
    #ifdef DEBUG_FS20_WS300
    debug_printf("received something via ws300, testing checksums...\n");
    #endif

    volatile struct ws300_datagram_t *d = &fs20_global.ws300.datagram;

    /* check markers, must all be 1 */
    if (! (d->p1 && d->p2 && d->p3 && d->p4 && d->p5 && d->p6 && d->p7 && d->p8 &&
           d->p9 && d->p10 && d->p11 && d->p12 && d->p13 && d->p14 && d->p15)) {
        #ifdef DEBUG_FS20_WS300
        debug_printf("at least one marker is not 1!\n");
        #endif

        return;
    }

    /* test constant */
    if (d->constant != FS20_WS300_CONSTANT) {
        #ifdef DEBUG_FS20_WS300
        debug_printf("invalid constant: %02x!\n", d->constant);
        #endif

        return;
    }

    /* test checksums */
    uint8_t xor, sum;

    xor = d->constant ^ d->flags ^ d->temp1 ^ d->temp2 ^ d->temp3
        ^ d->hygro1 ^ d->hygro2 ^ d->wind1 ^ d-> wind2 ^ d-> wind3
        ^ d->raincounter1 ^ d->raincounter2 ^ d->raincounter3
        ^ d->unknown ^ d->checksum1;

    sum = d->constant + d->flags + d->temp1 + d->temp2 + d->temp3
        + d->hygro1 + d->hygro2 + d->wind1 + d-> wind2 + d-> wind3
        + d->raincounter1 + d->raincounter2 + d->raincounter3
        + d->unknown + d->checksum1;

    /* 5 ist a strange magical constant =) */
    sum = (sum+5) & 0x0f;

    if (xor != 0) {
        #ifdef DEBUG_FS20_WS300
        debug_printf("invalid checksum1!\n");
        #endif

        return;
    }

    if (sum != d->checksum2) {
        #ifdef DEBUG_FS20_WS300
        debug_printf("invalid checksum2: %u != %u!\n", sum, d->checksum2);
        #endif

        return;
    }

    /* valid datagram, update global data */
    fs20_global.ws300.temp_frac = d->temp1;
    fs20_global.ws300.temp = d->temp2 + 10 * d->temp3;

    fs20_global.ws300.rain = (d->flags & _BV(FS20_WS300_FLAG_WATER)) > 0;

    if (d->flags & _BV(FS20_WS300_FLAG_TEMP))
        fs20_global.ws300.temp = -fs20_global.ws300.temp;

    fs20_global.ws300.hygro = d->hygro1 + 10 * d->hygro2;

    fs20_global.ws300.wind_frac = d->wind1;
    fs20_global.ws300.wind = d->wind2 + 10 * d->wind3;

    fs20_global.ws300.rain_value = d->raincounter1
                            + 10 * d->raincounter2
                           + 100 * d->raincounter3;


    #ifdef DEBUG_FS20_WS300
    debug_printf("new ws300 values: %u.%u deg, %u%% hygro, %u.%u km/h wind, ",
            fs20_global.ws300.temp,
            fs20_global.ws300.temp_frac,
            fs20_global.ws300.hygro,
            fs20_global.ws300.wind,
            fs20_global.ws300.wind_frac);
    if (fs20_global.ws300.rain)
        debug_printf("rain, ");

    debug_printf("rain counter: %u\n", fs20_global.ws300.rain_value);

    #endif


} /* }}} */
#endif

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
    memset((void *)&fs20_global.fs20.datagram, 0, sizeof(fs20_global.fs20));

    /* configure port pin for use as input to the analoge comparator */
    FS20_SEND_DDR &= ~_BV(FS20_RECV_PINNUM);
    FS20_SEND_PORT &= ~_BV(FS20_RECV_PINNUM);

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

#ifdef FS20_SUPPORT_RECEIVE_WS300

    /* reset everything to zero */
    memset((void *)&fs20_global.ws300, 0, sizeof(fs20_global.ws300));
#endif
#endif

#ifdef FS20_RECV_PROFILE
    fs20_global.int_counter = 0;
    fs20_global.ovf_counter = 0;
#endif
} /* }}} */

#endif
