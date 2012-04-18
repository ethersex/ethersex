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

#include "udpstella_net.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "services/stella/stella.h"
#include "core/portio/portio.h"
#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

/*
  STELLA_SET_IMMEDIATELY,
  STELLA_SET_FADE,
  STELLA_SET_FLASHY,
  STELLA_SET_IMMEDIATELY_RELATIVE,
  STELLA_SET_FADESTEP,
  STELLA_GETALL
*/

struct udpstella_packet
{
	uint8_t type;    // see above
	uint8_t channel; // if port: pin
	uint8_t value;
};

/* EXAMPLES:

Fade the first 3 channels of stella to 0 (consists of 3*3 bytes):
STELLA_SET_FADE + 0 + 0
STELLA_SET_FADE + 1 + 0
STELLA_SET_FADE + 2 + 0

Get the current channel values:
STELLA_GETALL + 0 + 0
The answer for 3 channels will be something like this:
stella + 3 + 0 + 0 + 0

*/

struct udpstella_answer_packet
{
	uint8_t type;
	uint8_t* data;
};

void
udpstella_net_init (void)
{
    uip_ipaddr_t ip;
    uip_ipaddr_copy (&ip, all_ones_addr);

    uip_udp_conn_t *udp_echo_conn = uip_udp_new (&ip, 0, udpstella_net_main);

    if (!udp_echo_conn) 
	return; /* dammit. */

    uip_udp_bind (udp_echo_conn, HTONS (UDP_STELLA_PORT));
}


void
udpstella_net_main(void)
{
    if (!uip_newdata ())
	return;

    uip_slen = 0;
    uint16_t len = uip_len;
    uint8_t buffer[uip_len];
    memcpy(buffer, uip_appdata, uip_len);

    struct udpstella_packet* packet = (struct udpstella_packet*)buffer;
    uint8_t* answer = uip_appdata;

    while (len>=sizeof(struct udpstella_packet)) {
	    if (packet->type == STELLA_GETALL) {
		answer[0] = 's';
		answer[1] = 't';
		answer[2] = 'e';
		answer[3] = 'l';
		answer[4] = 'l';
		answer[5] = 'a';
		answer[6] = (uint8_t)STELLA_CHANNELS;
		for (uint8_t c=0;c<STELLA_CHANNELS;++c) {
			answer[7+c] = stella_brightness[c];
		}
		uip_slen += STELLA_CHANNELS+7;
		answer += STELLA_CHANNELS+7;
	    } else if (packet->type == STELLA_SET_FADESTEP) {
	    	stella_setFadestep(packet->value);
	    } else {
		stella_setValue(packet->type, packet->channel, packet->value);
	    }
     	packet++;
     	len-=sizeof(struct udpstella_packet);
     }

    if (uip_slen == 0) return;
	/* Sent data out */

	uip_udp_conn_t echo_conn;
	uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
	echo_conn.rport = BUF->srcport;
	echo_conn.lport = HTONS(UDP_STELLA_PORT);

	uip_udp_conn = &echo_conn;
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;

}

/*
  -- Ethersex META --
  header(protocols/udpstella/udpstella_net.h)
  net_init(udpstella_net_init)
*/
