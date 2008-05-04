/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/interrupt.h>

#include "timer.h"
#include "config.h"
#include "network.h"
#include "debug.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/uip_neighbor.h"
#include "fs20/fs20.h"
#include "watchcat/watchcat.h"
#include "clock/clock.h"
#include "cron/cron.h"
#include "ipv6.h"
#include "stella/stella.h"
#include "ps2/ps2.h"
#include "rfm12/rfm12.h"
#include "syslog/syslog.h"

#ifdef BOOTLOADER_SUPPORT
uint8_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

#ifdef RFM12_SUPPORT
unsigned short rfm12_t_status = 0;
extern uint8_t RFM12_akt_status;
#endif

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

#       ifdef  WATCHCAT_SUPPORT
        watchcat_periodic();
#       endif

#       if UIP_CONNS <= 255
        uint8_t i;
#       else
        uint16_t i;
#       endif

        /* process fs20 stuff */
#       ifdef FS20_SUPPORT
#       ifdef FS20_SUPPORT_RECEIVE
        fs20_process_timeout();
#       endif

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
#       endif /* FS20_SUPPORT */

        
#       ifdef PS2_SUPPORT
        ps2_periodic();
#       endif

        /* check tcp connections every 200ms */
#       ifdef TEENSY_SUPPORT
        if ((counter & 7) == 0) {
#       else
        if (counter % 10 == 0) {
#       endif

#       ifdef ZBUS_SUPPORT
          zbus_core_periodic();
#       endif /* ZBUS_SUPPORT */

#           if UIP_TCP == 1
            for (i = 0; i < UIP_CONNS; i++) {
		uip_stack_set_active(uip_conns[i].stack);
                uip_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0)
		    fill_llh_and_transmit();
            }
#           endif /* UIP_TCP == 1 */

#           if UIP_UDP == 1
            /* check udp connections every time */
            for (i = 0; i < UIP_UDP_CONNS; i++) {
		uip_stack_set_active(uip_udp_conns[i].stack);
                uip_udp_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0)
		    fill_llh_and_transmit();
            }
#           endif
        }

#       if UIP_CONF_IPV6 && defined(ENC28J60_SUPPORT)
        if (counter == 5) { 
            /* Send a router solicitation every 10 seconds, as long
               as we only got a link local address.  First time one
               second after boot */
#           ifdef OPENVPN_SUPPORT		
	    uip_stack_set_active(STACK_OPENVPN);
#           endif
#       ifndef IPV6_STATIC_SUPPORT
            if(((u16_t *)(uip_hostaddr))[0] == HTONS(0xFE80)) {
                uip_router_send_solicitation();
                transmit_packet();
            }
#       endif
        }
#       endif /* UIP_CONF_IPV6 and ENC28J60_SUPPORT */

        if (counter % 50 == 0) {

#           ifdef CRON_SUPPORT
            cron_periodic();
#           endif

#           ifdef FS20_SUPPORT
            fs20_global.ws300.last_update++;
#           endif

#           ifdef NTP_SUPPORT
#           ifdef OPENVPN_SUPPORT
	    uip_stack_set_active(STACK_OPENVPN);
#           endif
            clock_tick();
	    if (uip_len)
		fill_llh_and_transmit();
#           endif /* NTP_SUPPORT */
        }
#       ifdef RFM12_SUPPORT
        if (counter == 300 ) {
          rfm12_t_status = rfm12_trans(0x0000); /*get the status Register from the RFM12*/
#         ifdef SYSLOG_SUPPORT
          syslog_sendf("timer rfm12_trans: %04X %02X\n", rfm12_t_status, RFM12_akt_status);
#         endif
          /* FIXME do anything when rfm12 kommunication hangs */
        }
#       endif
        /* expire arp entries every 10 seconds */
        if (counter == 500) {
#           ifdef DEBUG_TIMER
            debug_printf("timer: 10 seconds have passed, expiring arp entries\n");
#           endif

#           ifdef ENC28J60_SUPPORT
#	    ifdef RFM12_BEACON_SUPPORT
            /* send beacon id packet. */
	    uip_buf[0] = 0x23;
	    uip_buf[1] = rfm12_beacon_code;
	    uip_buf[2] = 0x42;
            rfm12_txstart (uip_buf, 3);
#           endif /* RFM12_BEACON_SUPPORT */

#           ifndef BOOTLOADER_SUPPORT
#           if UIP_CONF_IPV6
            uip_neighbor_periodic();
#           else
            uip_arp_timer();
#           endif
#           endif /* !BOOTLOADER_SUPPORT */
#           endif /* ENC28J60_SUPPORT */

            counter = 0;
        }

#       ifdef BOOTLOADER_SUPPORT
        if(bootload_delay)
            if(-- bootload_delay == 0) {
		cli();
		_IVREG = _BV(IVCE);	        /* prepare ivec change */
		_IVREG = 0x00;                  /* change ivec */

                void (*jump_to_application)(void) = NULL;
                jump_to_application();
            }
#       endif

        /* clear flag */
        _TIFR_TIMER1 = _BV(OCF1A);
    }

} /* }}} */
