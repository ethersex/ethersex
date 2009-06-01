/*
 * Copyright (C) 2007, 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#include <string.h>
#include "uecmd_net.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/debug.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/speed_parser.h"

#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void uecmd_net_init() {
	uip_ipaddr_t ip;
	uip_ipaddr_copy(&ip, all_ones_addr);

	uip_udp_conn_t *uecmd_conn = uip_udp_new(&ip, 0, uecmd_net_main);
	if(! uecmd_conn) {
		debug_printf("ecmd: udp failed\n");
		return;
	}

	uip_udp_bind (uecmd_conn, HTONS(ECMD_UDP_PORT));
}

void uecmd_net_main() {
	if (!uip_newdata ())
		return;

	/* The udp interface for ecmd provides two modes:
	 * a) normal ecmd parse mode, default
	 * b) speed mode: one byte command, payload, ...
	 *    initiate with newline as first character */

	char *p = (char *)uip_appdata;
	#ifdef ECMD_SPEED_SUPPORT
	if (*p == '\n') // speed mode
	{
		ecmd_speed_parse((void*)++p, uip_datalen()-1);
		#ifdef EBCMD_RESPONSE_ACK
		ebcmd_net_ack();
		#endif
		return;
	}
	#endif

	/* Add \0 to the data and remove \n from the data */
	do {
		if (*p == '\r' || *p == '\n') {
			break;
		}
	} while ( ++p <= ((char *)uip_appdata + uip_datalen()));

	/* Parse the Data */
	*p = 0;
	char cmd[p - (char *)uip_appdata];

	strncpy(cmd, uip_appdata, p - (char *)uip_appdata + 1);

	uip_slen = 0;
	while (uip_slen < UIP_BUFSIZE - UIP_IPUDPH_LEN) {
		int16_t len = ecmd_parse_command(cmd, ((char *)uip_appdata) + uip_slen,
											(UIP_BUFSIZE - UIP_IPUDPH_LEN) - uip_slen);
		uint8_t real_len = len;
		if (len < 0) {
			real_len = (uint8_t)  -len - 10;
		}
		uip_slen += real_len + 1;
		((char *)uip_appdata)[uip_slen - 1] = '\n';
		if (real_len == len || len == 0)
			break;
	}

	/* Sent data out */

	uip_udp_conn_t echo_conn;
	uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
	echo_conn.rport = BUF->srcport;
	echo_conn.lport = HTONS(ECMD_UDP_PORT);

	uip_udp_conn = &echo_conn;
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;
}

/*
  -- Ethersex META --
  header(protocols/ecmd/via_udp/uecmd_net.h)
  net_init(uecmd_net_init)
*/
