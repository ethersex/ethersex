/*
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

#include <avr/pgmspace.h>

#include "../uip/uip.h"
#include "../net/tftp_net.h"
#include "../tftp/tftp.h"
#include "tftp_net.h"
#include "tftp_state.h"

void
tftp_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);

    struct uip_udp_conn *tftp_conn = uip_udp_new(&ip, 0);

    if(! tftp_conn) 
	return;					/* dammit. */

    uip_udp_bind(tftp_conn, HTONS(TFTP_PORT));

    tftp_conn->appstate.tftp.fire_req = 0;
}


void
tftp_net_main(void)
{
    if(uip_newdata()) {
	if(uip_udp_conn->lport == HTONS(TFTP_ALT_PORT)) {
	    /* got reply packet for tftp download request,
	     * seek request connection and shut it down */
	    struct uip_udp_conn *req_conn;
	    for(req_conn = &uip_udp_conns[0];
		req_conn < &uip_udp_conns[UIP_UDP_CONNS];
		++ req_conn) {
		if(req_conn->lport != HTONS(TFTP_ALT_PORT))
		    continue;
		if(req_conn->rport != HTONS(TFTP_PORT))
		    continue;

		req_conn->lport = 0;		/* clear connection */
		break;
	    }
	}

	tftp_handle_packet();
	return;
    }

    if(! uip_udp_conn->appstate.tftp.fire_req)
	return;

    if(uip_udp_conn->appstate.tftp.transfered) {
	uip_udp_conn->appstate.tftp.transfered --;
	return;
    }

    /*
     * fire download request packet ...
     */
    struct tftp_hdr *tftp_pk = uip_appdata;
    tftp_pk->type = HTONS(1);			/* read request */
    int l = strlen(uip_udp_conn->appstate.tftp.filename);
    memcpy(tftp_pk->u.raw, uip_udp_conn->appstate.tftp.filename, l + 1);
    memcpy_P(&tftp_pk->u.raw[l + 1], PSTR("octet"), 6);
    uip_udp_send(l + 9);

    /* uip_udp_conn->appstate.tftp.fire_req = 0; */
    uip_udp_conn->appstate.tftp.transfered = 5; /* retransmit in 2.5 seconds */
}

