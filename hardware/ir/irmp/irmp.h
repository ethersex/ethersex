/*
 *         simple irmp implementation
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


#ifndef IRMP_H
#define IRMP_H

/* configuration:
 *
 * if you want to use this library, define IRMP_SUPPORT somewhere.
 * pin defines (eg for using PD4 for sending):
 *
 * #define IRMP_SEND_PINNUM PD4
 * #define IRMP_SEND_PORT PORTD
 * #define IRMP_SEND_DDR DDRD
 *
 * for receive:
 *
 * #define IRMP_QUEUE_LENGTH 10
 *
 */

#include <stdint.h>

#ifdef IRMP_SUPPORT

#if 0
#define IRMP_COUNTERS 64

/* structures */
struct irmp_t {
    union {
        uint16_t raw;
        struct {
            uint8_t code:6;             /* first 6 bits: control code */
            uint8_t address:5;          /* next 5 bits: address */
            uint8_t toggle_bit:1;       /* next bit is the toggle bit */
            uint8_t spare:4;            /* spare bits */
        };
    };
#ifdef IRMP_UDP_SUPPORT
    uint8_t bitcount;
    uint8_t cnt[IRMP_COUNTERS];
#endif
};

struct irmp_global_t {
    struct irmp_t received_command;
    uint8_t enabled;                /* if one, decoder is active */
    uint8_t new_data;               /* if one, new data is available */
    uint8_t halfbitcount;
    uint8_t interrupts;
    uint8_t temp_disable;           /* disable decoder, used internally! */
    struct irmp_t queue[IRMP_QUEUE_LENGTH];
    uint8_t len;
#ifdef IRMP_UDP_SUPPORT
    /* network extension */
    uint8_t bitcount;
    uint8_t cnt[IRMP_COUNTERS];
    uint8_t disablelog;
#endif
};

extern volatile struct irmp_global_t irmp_global;

/* timing constants */

/* one pulse half is 889us, for _delay_loop_2 */
#define IRMP_PULSE (F_CPU / 1000000 * 888 / 4)


/* UDP constants */
#define IRMP_UDPPORT 6669
#endif

/* prototypes */
void irmp_init(void);
#ifdef IRMP_UDP_SUPPORT
void irmp_net_init(void);
void irmp_udp_send(void);
void irmp_udp_recv(void);
uint8_t irmp_check_cache(void);
#endif
void irmp_send(uint8_t addr, uint8_t cmd);
void irmp_process(void);

#endif /* IRMP_SUPPORT */
#endif /* IRMP_H */
