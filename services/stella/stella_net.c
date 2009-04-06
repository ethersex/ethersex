/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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
 */

#include "uip/uip.h"
#include "uip/uip_router.h"
#include "config.h"
#include "debug.h"
#include "services/cron/cron.h"
#include "stella.h"
#include "stella_net.h"
#include "stella_protocol.h"

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
}


void
stella_net_main(void)
{
	if (!uip_newdata ())
		return;

	stella_protocol_parse(uip_appdata, uip_len);

	#ifdef STELLA_RESPONSE_ACK
	stella_net_ack();
	#endif
}

#ifdef STELLA_RESPONSE

#ifdef STELLA_RESPONSE_ACK
// send a response message (header+copy of original msg)
inline void
stella_net_ack(void)
{
	// move original message 3 bytes ahead
	memmove(uip_appdata+3,uip_appdata,uip_len);
	char* response = uip_appdata;
	response[0] = 'S';
	response[1] = STELLA_ACK_RESPONSE;
	response[2] = uip_len;
	stella_net_unicast(uip_len+3);
}
#endif

void *
stella_net_response(const uint8_t msgtype)
{
	struct stella_response_header *r = uip_appdata;
	r->protocol = 'S';
	r->cmd = msgtype;
	void *p = r;
	p += 2;
	return p;
}

void
stella_net_wb_getvalues(const uint8_t msgtype)
{
	struct stella_response_detailed_struct *r;
	r = stella_net_response(msgtype);

	r->protocol_version = STELLA_PROTOCOL_VERSION;
	r->channel_count = STELLA_PINS;

	/* copy the pwm channel values */
	memcpy(r->pwm_channels, stella_brightness, sizeof(r->pwm_channels));
}

void stella_net_unicast(uint8_t len)
{
	uip_udp_send(len);

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

void stella_net_broadcast(uint8_t len)
{
	uip_udp_send(len);

	/* create broadcast ip v4 address; TODO: ip v6 */
	uip_ipaddr_t addr;
	uip_ipaddr(&addr, 255,255,255,255);

	/* Send the packet */
	uip_udp_conn_t conn;
	uip_ipaddr_copy(conn.ripaddr, addr);
	conn.rport = BUF->srcport;
	conn.lport = HTONS(STELLA_UDP_PORT);

	uip_udp_conn = &conn;

	/* Send immediately */
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;
}
#endif

#endif /* STELLA_SUPPORT */
