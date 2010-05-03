/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2009 by David Gräff
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

#include "udpio_net.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/portio/portio.h"
#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

struct udpio_packet
{
  /* Port, where 0=PORTA, ..., 3=PORTD
     255=get all pins of all ports
  */
	uint8_t port;
  /* Pins: Function depends on "nstate". Every bit
     corresponds to one pin, where the most significant bit
     means pin 0 of the port selected above.
     if nstate is 0: (disable)
       Disables all pins, where the corresponding bit of "pins"
       is set to 0.
     if nstate is 1: (enable)
       Enables all pins, where the corresponding bit of "pins"
       is set to 1.
     if nstate is 2: (set)
       Set the port to the value of "pins".
  */
	uint8_t pins;
	uint8_t nstate;
};

void
udpio_net_init (void)
{
    uip_ipaddr_t ip;
    uip_ipaddr_copy (&ip, all_ones_addr);

    uip_udp_conn_t *udp_echo_conn = uip_udp_new (&ip, 0, udpio_net_main);

    if (!udp_echo_conn) 
	return; /* dammit. */

    uip_udp_bind (udp_echo_conn, HTONS (UDP_IO_PORT));
}


void
udpio_net_main(void)
{
    if (!uip_newdata ())
	return;

    uip_slen = 0;
    uint16_t len = uip_len;
    uint8_t buffer[uip_len];
    memcpy(buffer, uip_appdata, uip_len);

    struct udpio_packet* packet = (struct udpio_packet*)buffer;
    uint8_t* answer = uip_appdata;

    while (len>=sizeof(struct udpio_packet))
	{
		/* Get values */
	    if (packet->port == 255)
		{
			answer[0] = 'p';
			answer[1] = 'i';
			answer[2] = 'n';
			answer[3] = 's';
			answer[4] = PORTA;
			answer[5] = PORTB;
			answer[6] = PORTC;
			answer[7] = PORTD;
			uip_slen += 8;
			answer += 8;
	    }
		/* Set port to "pins" value */
		else if (packet->nstate == 2)
		{
			if (packet->port > IO_HARD_PORTS) break;
			vport[packet->port].write_port(packet->port, packet->pins);
		}
		/* Enables pins */
	    else if (packet->nstate == 1)
		{
			if (packet->port > IO_HARD_PORTS) break;
			vport[packet->port].write_port(packet->port, vport[packet->port].read_port(packet->port) | packet->pins);
	    }
		/* Disables pins */
	    else if (packet->nstate == 0) {
			if (packet->port > IO_HARD_PORTS) break;
			vport[packet->port].write_port(packet->port, vport[packet->port].read_port(packet->port) & ~(uint8_t)packet->pins);
	    }
     	packet++;
     	len-=sizeof(struct udpio_packet);
     }

    if (uip_slen == 0) return;
	/* Sent data out */

	uip_udp_conn_t echo_conn;
	uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
	echo_conn.rport = BUF->srcport;
	echo_conn.lport = HTONS(UDP_IO_PORT);

	uip_udp_conn = &echo_conn;
	uip_process(UIP_UDP_SEND_CONN);
	router_output();

	uip_slen = 0;

}

/*
  -- Ethersex META --
  header(protocols/udpIO/udpio_net.h)
  net_init(udpio_net_init)
*/
