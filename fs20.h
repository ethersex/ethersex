/* vim:fdm=marker ts=4 et ai
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


#ifndef _FS20_H
#define _FS20_H

#include "config.h"

#ifdef FS20_SUPPORT

#ifdef FS20_SUPPORT_SEND
    #if !defined(FS20_SEND_PINNUM) || !defined(FS20_SEND_DDR) || !defined(FS20_SEND_PORT)
    #error "FS20_SEND_PINNUM, FS20_SEND_DDR or FS20_SEND_PORT not defined!"
    #endif
#endif

#ifdef FS20_SUPPORT_RECEIVE
    #if !defined(FS20_RECV_PINNUM) || !defined(FS20_RECV_DDR) || !defined(FS20_RECV_PORT)
    #error "FS20_RECV_PINNUM, FS20_RECV_DDR or FS20_RECV_PORT not defined!"
    #endif
#endif

#if !defined(F_CPU)
#error "F_CPU undefined!"
#endif

#define FS20_DELAY_ZERO (4 * (F_CPU / 10000) / 4) /* 400uS, for delay_loop_2 */
#define FS20_DELAY_ONE  (6 * (F_CPU / 10000) / 4) /* 600uS, for delay_loop_2 */
#define FS20_DELAY_CMD  ( F_CPU / 10000 / 4) /* 10ms, for delay_loop_2 */

#define FS20_BETWEEN(x, a, b) ((x >= a) && (x <= b))
#define FS20_SYMM(x, y, r) ( (x-y) <= r && (y-x) <= r )

/* zero is 400uS: F_CPU/10^6 * 400 = 8000, with prescaler 128 gives 62.5
 * -> test if value is between 33 and 73 */
#define FS20_PULSE_ZERO(x) FS20_BETWEEN((x), 33, 73)
/* one is 600uS: F_CPU/10^6 * 600 = 12000, with prescaler 128 gives 93.75
 * -> test if value is between 74 and 105 */
#define FS20_PULSE_ONE(x) FS20_BETWEEN((x), 74, 105)
/* maximal difference between two pulses is 115.2 uS,
 * which means 18 timer cycles with prescaler 128 */
#define FS20_PULSE_DIFFERENCE(x,y) FS20_SYMM(x, y, 68)

/* a datagram consists of 58 bits */
#define FS20_DATAGRAM_LENGTH 58

/* queue length */
#define FS20_QUEUE_LENGTH 5

/* structures */
struct fs20_datagram_t {
    uint8_t p5:1;
    uint8_t parity:8;
    uint8_t p4:1;
    uint8_t cmd:8;
    uint8_t p3:1;
    uint8_t addr:8;
    uint8_t p2:1;
    uint8_t hc2:8;
    uint8_t p1:1;
    uint8_t hc1:8;
    uint16_t sync:13;
};

struct fs20_global_t {
    uint8_t enable;
    #ifdef FS20_SUPPORT_RECEIVE
    union {
        struct fs20_datagram_t datagram;
        uint64_t raw;
        uint16_t words[4];
    };
    uint8_t rec;
    uint8_t err;
    struct fs20_datagram_t queue[FS20_QUEUE_LENGTH];
    uint8_t len;
    uint8_t timeout;
    #endif
    #ifdef FS20_RECV_PROFILE
    uint16_t int_counter;
    uint16_t ovf_counter;
    #endif
};

/* global variables */
extern volatile struct fs20_global_t fs20_global;

/* public prototypes */
void fs20_init(void);

#ifdef FS20_SUPPORT_SEND
void fs20_send(uint16_t housecode, uint8_t address, uint8_t command);
#endif

#ifdef FS20_SUPPORT_RECEIVE
void fs20_process(void);
void fs20_process_timeout(void);
#else
#define fs20_process()
#define fs20_process_timeout()
#endif

#endif /* FS20_SUPPORT */

#endif /* FS20_H */
