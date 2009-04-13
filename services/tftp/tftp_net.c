/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "protocols/uip/uip.h"
#include "tftp.h"
#include "tftp_net.h"
#include "tftp_state.h"

void
tftp_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr_copy(&ip, all_ones_addr);

    uip_udp_conn_t *tftp_conn = uip_udp_new(&ip, 0, tftp_net_main);

    if(! tftp_conn)
	return;			/* dammit. */

    uip_udp_bind(tftp_conn, HTONS(TFTP_PORT));

#ifdef TFTPOMATIC_SUPPORT
    tftp_conn->appstate.tftp.fire_req = 0;

#ifndef IPV6_SUPPORT		/* IPv6 is handled in ipv6.c (after ra) */
    const unsigned char *filename = CONF_TFTP_IMAGE;
    CONF_TFTP_IP;

    tftp_fire_tftpomatic(&ip, filename);
#endif /* !IPV6_SUPPORT */
#endif /* TFTPOMATIC_SUPPORT */
}


void
tftp_net_main(void)
{
    if(uip_newdata()) {
#ifdef BOOTLOADER_SUPPORT
	if(uip_udp_conn->lport == HTONS(TFTP_ALT_PORT)) {
	    /* got reply packet for tftp download request,
	     * seek request connection and shut it down */
	    uip_udp_conn_t *req_conn;
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
#endif /* BOOTLOADER_SUPPORT */

	tftp_handle_packet();
	return;
    }

#if defined(BOOTLOADER_SUPPORT)  \
  && (defined(TFTPOMATIC_SUPPORT) || defined(BOOTP_SUPPORT))
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
#endif
}

/*
  -- Ethersex META --
  header(services/tftp/tftp_net.h)
  net_init(tftp_net_init)
*/
