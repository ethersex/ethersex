/* vim:fdm=marker ts=4 et ai
 * {{{
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

#include "timer.h"
#include "config.h"
#include "network.h"
#include "debug.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "fs20/fs20.h"

void timer_init(void)
/* {{{ */ {

    /* init timer1 to expire after ~20ms, with CTC enabled */
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);
    OCR1A = (F_CPU/1024/50);

} /* }}} */

void timer_process(void)
/* {{{ */ {

    static uint16_t counter = 0;

    /* check timer 1 (timeout after 50ms) */
    if (_TIFR_TIMER1 & _BV(OCF1A)) {

        counter++;

#       ifdef DEBUG_TIMER
        debug_printf("timer: counter is %d\n", counter);
#       endif

#       if UIP_CONNS <= 255
        uint8_t i;
#       else
        uint16_t i;
#       endif

        /* process fs20 stuff */
        fs20_process_timeout();

#       ifdef FS20_RECV_PROFILE
        /* output fs20 profiling information */
        if (counter % 10 == 0) {
            uint16_t c1 = fs20_global.int_counter;
            uint16_t c2 = fs20_global.ovf_counter;
            fs20_global.int_counter = 0;
            fs20_global.ovf_counter = 0;
            debug_printf("fs20 profile: %u %u\n", c1, c2);
        }
#       endif

        /* check tcp connections every 200ms */
        if (counter % 10 == 0) {
            for (i = 0; i < UIP_CONNS; i++) {
                uip_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0) {
                    uip_arp_out();
                    transmit_packet();
                }
            }

#           if UIP_UDP == 1
            /* check udp connections every time */
            for (i = 0; i < UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0) {
                    // XXX FIXME if (uip_arp_out() == 0)
                    // XXX FIXME     uip_udp_conn->appstate.sntp.transmit_state = 1;

                    transmit_packet();
                }
            }
#           endif
        }

        // FIXME
        //if (c % 5 == 0) /* every second */
        //    clock_periodic();

        /* expire arp entries every 10 seconds */
        if (counter == 500) {
#           ifdef DEBUG_TIMER
            debug_printf("timer: 10 seconds have passed, expiring arp entries\n");
#           endif

            uip_arp_timer();
            counter = 0;
        }

        /* clear flag */
        _TIFR_TIMER1 = _BV(OCF1A);
    }

} /* }}} */
