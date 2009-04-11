/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
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

#include "udp_echo_net.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
udp_echo_net_init (void)
{
    uip_ipaddr_t ip;
    uip_ipaddr_copy (&ip, all_ones_addr);

    uip_udp_conn_t *udp_echo_conn = uip_udp_new (&ip, 0, udp_echo_net_main);

    if (!udp_echo_conn) 
	return; /* dammit. */

    uip_udp_bind (udp_echo_conn, HTONS (ECHO_PORT));
}


void
udp_echo_net_main(void)
{
    if (!uip_newdata ())
	return;

    uip_udp_conn_t echo_conn;
    uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
    echo_conn.rport = BUF->srcport;
    echo_conn.lport = HTONS(ECHO_PORT);

    uip_udp_conn = &echo_conn;

    uip_slen = uip_len;
    uip_process(UIP_UDP_SEND_CONN); 
    router_output();

    uip_slen = 0;
}

/*
  -- Ethersex META --
  header(services/echo/udp_echo_net.h)
  net_init(udp_echo_net_init)
*/
