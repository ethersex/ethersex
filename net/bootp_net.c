/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
 */

/* we want to live on the outer stack, if there are two ... */
#include "../uip/uip_openvpn.h"

#include <stdlib.h>

#include "bootp_net.h"
#include "bootp_state.h"
#include "../bootp/bootp.h"
#include "../uip/uip.h"
#include "../config.h"

#ifdef BOOTP_SUPPORT

void
bootp_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);

    struct uip_udp_conn *bootp_conn = uip_udp_new(&ip, HTONS(BOOTPS_PORT), bootp_net_main);

    if(! bootp_conn) 
	return; /* dammit. */

    uip_udp_bind(bootp_conn, HTONS(BOOTPC_PORT));

    bootp_conn->appstate.bootp.configured = 0;
    bootp_conn->appstate.bootp.retry_timer = 0;
}


void
bootp_net_main(void)
{
    if(uip_newdata())
	bootp_handle_reply();

    if(uip_udp_conn->appstate.bootp.configured)
	return;				        /* no more queries needed */

    if(! uip_udp_conn->appstate.bootp.retry_timer) {
	bootp_send_request();

	if(uip_udp_conn->appstate.bootp.retry_counter < 5)
	    uip_udp_conn->appstate.bootp.retry_timer = 
		2 << (++ uip_udp_conn->appstate.bootp.retry_counter);
	else
	    uip_udp_conn->appstate.bootp.retry_timer = 64 + (rand() & 63);
    }
    else
	uip_udp_conn->appstate.bootp.retry_timer --;
}

#endif /* BOOTP_SUPPORT */
