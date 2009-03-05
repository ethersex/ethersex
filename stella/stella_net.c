/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include "../uip/uip.h"
#include "../uip/uip_router.h"
#include "../config.h"
#include "../debug.h"
#include "../stella/stella.h"
#include "stella_net.h"

#ifdef STELLA_SUPPORT
#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
stella_net_init(void)
{
	uip_ipaddr_t ip;
	uip_ipaddr_copy(&ip, all_ones_addr);

	uip_udp_conn_t *stella_conn = uip_udp_new(&ip, 0, stella_net_main);

	if(! stella_conn) {
		debug_printf("syslog: couldn't create connection\n");
		return;
	}

	uip_udp_bind (stella_conn, HTONS(STELLA_UDP_PORT));
	stella_pwm_init ();

	#ifdef DEBUG_STELLA
	debug_printf("Stella initalized\n");
	#endif
}

void
stella_net_main(void)
{
	if (!uip_newdata ())
		return;

	#ifdef DEBUG_STELLA
	debug_printf("Stella received packet\n");
	#endif

	/* if received package is only 1 byte of size, it has to be one of the
	respond commands */
	if (uip_len == 1)
	{
		#ifdef STELLA_RESPONSE
		if (((unsigned char*)uip_appdata)[0] == STELLA_UNICAST_RESPONSE)
		{
			stella_net_unicast_response();
		}
		#endif
		#ifdef STELLA_RESPONSE_BROADCAST
		if (((unsigned char*)uip_appdata)[0] == STELLA_BROADCAST_RESPONSE)
		{
			stella_net_broadcast_response();
		}
		#endif
	}
	/* ack this packet if the STELLA_ACK_RESPONSE command was set */
	else
	if (stella_process (uip_appdata, uip_len) & STELLA_FLAG_ACK)
	{
		#ifdef STELLA_RESPONSE_ACK
		stella_net_ack_response();
		#endif
	}

	#ifdef DEBUG_STELLA
	debug_printf("Stella processed packet\n");
	#endif
}

#ifdef STELLA_RESPONSE
void
stella_net_unicast_response(void) {
	#ifdef DEBUG_STELLA
	debug_printf("Stella unicast response\n");
	#endif
	struct stella_response_struct *response_packet = uip_appdata;

	/* identifier */
	response_packet->identifier = 'S';

	/* copy the pwm channel values */
	memcpy(response_packet->pwm_channels, stella_color,
		sizeof(response_packet->pwm_channels));

	uip_udp_send(sizeof(struct stella_response_struct));

	/* Send the packet */
	uip_udp_conn_t conn;
	uip_ipaddr_copy(conn.ripaddr, BUF->srcipaddr);
	conn.rport = BUF->srcport;
	conn.lport = HTONS(STELLA_UDP_PORT);

	uip_udp_conn = &conn;

	/* Send immediately */
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;
}
#endif /* STELLA_RESPONSE */

#ifdef STELLA_RESPONSE_BROADCAST
void
stella_net_broadcast_response(void) {
	#ifdef DEBUG_STELLA
	debug_printf("Stella broadcast response\n");
	#endif

	struct stella_response_struct *response_packet = uip_appdata;

	/* identifier */
	response_packet->identifier = 'S';

	/* copy the pwm channel values */
	memcpy(response_packet->pwm_channels, stella_color,
		sizeof(response_packet->pwm_channels));

	/* create another udp connection (we are currently in one!)
	for broadcasting. Let uip call us, send the packet. */

	// TODO EXPERIMENTAL CODE, NOT TESTED. MAY HARM YOUR PETS ETC
	// build issues (gcc):
        // "warning: assignment from incompatible pointer type"
	struct uip_udp_conn *c;
	uip_ipaddr_t addr;
	uip_ipaddr(&addr, 255,255,255,255);
	c = uip_udp_new(&addr, STELLA_BROADCAST_UDP_PORT, NULL);

	uip_udp_periodic_conn(c);

	uip_udp_send(sizeof(struct stella_response_struct));

	/* Send immediately */
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;
}
#endif /* STELLA_BROADCAST_RESPONSE */

#ifdef STELLA_RESPONSE_ACK
void
stella_net_ack_response(void) {
	#ifdef DEBUG_STELLA
	debug_printf("Stella ack packet\n");
	#endif
	/* ack response: two bytes. First: identifier 'S', Second: packet length */

	/* identifier and length of the received packet */
	char* response = uip_appdata;
	response[0] = 'S';
	response[1] = uip_len;
	uip_udp_send(2);

	/* Send the packet */
	uip_udp_conn_t conn;
	uip_ipaddr_copy(conn.ripaddr, BUF->srcipaddr);
	conn.rport = BUF->srcport;
	conn.lport = HTONS(STELLA_UDP_PORT);

	uip_udp_conn = &conn;

	/* Send immediately */
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;
}
#endif /* STELLA_RESPONSE_ACK */

#endif /* STELLA_SUPPORT */
