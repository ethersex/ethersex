/*
 * Authors: David Gräff <david.graeff-at-web.de> (2009-2011)
 * 
 * Purpose:
 * Allows pins to be set via simple udp packages as well as
 * requesting the current status of pins and addionally provides the
 * ability to register a client to be informed of pin changes.
 * This module allows very fast polling and
 * setting without much network overhead. Recommended
 * use only in a trusted network environment.
 *
 * Changes:
 * 29.09.2011: Getting port states is binary incompatible to the 2009 Version
 * of this module!
 *
 * Depends on:
 * ethersex port io abstraction, uip
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

struct udpio_notify_on_change {
	uip_ip4addr_t address;
	uint8_t pinport;
	uint8_t pinmask;
	uint8_t last_pin_state;
};
#define ALLOWED_CLIENTS 3
struct udpio_notify_on_change clients[ALLOWED_CLIENTS];

enum udpio_packet_mode
{
	udpIOOnlyDisable = 0,
	udpIOOnlyEnable = 1,
	udpIOEnableAndDisable = 2,
	udpIOGetAllPortPins = 3,
	udpIORegisterClientForPortChanges = 4, /* "pinmask & PORTx" are used to compare for changes! */
	udpIOUnRegisterClientForPortChanges = 5
};

struct udpio_packet
{
	/* Port: 0=PORTA, ..., 3=PORTD, ...
	*/
	uint8_t port;
	/* Pins: Every bit corresponds to one pin, where the least significant (right) bit
	means pin 0.
	*/
	uint8_t pinmask;
	uint8_t /*udpio_packet_mode*/ mode;
};

void
udpio_net_init (void)
{
	uip_ipaddr_t ip;
	uip_ipaddr_copy (&ip, all_ones_addr);

	uip_udp_conn_t *udp_io_connection = uip_udp_new (&ip, 0, udpio_net_main);

	if (!udp_io_connection) 
	return; /* dammit. */

	uip_udp_bind (udp_io_connection, HTONS (UDP_IO_PORT));
}


void
udpio_net_main(void)
{
	uint8_t* answer = uip_appdata;
	uint8_t i;
	uip_ipaddr_t nullip;
	uip_ipaddr(&nullip, 0,0,0,0);


	if (uip_newdata ()) {
		// No new data: We will compare pins instead
		for (i=0;i<ALLOWED_CLIENTS;++i) {
			if(uip_ipaddr_cmp(&(clients[i].address), &nullip))
				continue;
			// found address of calling client in clients: compare pins now
			if (vport[clients[i].pinport].read_port(clients[i].pinport) ^ clients[i].last_pin_state) {
				// update last pin state
				clients[i].last_pin_state = vport[clients[i].pinport].read_port(clients[i].pinport) & clients[i].pinmask;
				// send changes
				answer[0] = 'p';
				answer[1] = 'i';
				answer[2] = 'n';
				answer[3] = 'c';
				answer[4] = clients[i].pinport;
				answer[5] = clients[i].last_pin_state;
				uip_slen += 6;
				answer += 6;

				uip_udp_conn_t echo_conn;
				uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
				echo_conn.rport = HTONS(UDP_IO_PORT); // send packages to clients at the udp io module port
				echo_conn.lport = HTONS(UDP_IO_PORT);

				uip_udp_conn = &echo_conn;
				uip_process(UIP_UDP_SEND_CONN);
				router_output();

				uip_slen = 0;
			}
		}
		// No new data, return now
		return;
	}

	uip_slen = 0;
	uint16_t len = uip_len;
	uint8_t buffer[uip_len];
	memcpy(buffer, uip_appdata, uip_len);
	struct udpio_packet* packet = (struct udpio_packet*)buffer;

	while (len>=sizeof(struct udpio_packet))
	{
		switch(packet->mode) {
			case udpIOOnlyDisable:
				if (packet->port > IO_HARD_PORTS) break;
				vport[packet->port].write_port(packet->port, vport[packet->port].read_port(packet->port) & ~(uint8_t)packet->pinmask);
				break;
			case udpIOOnlyEnable:
				if (packet->port > IO_HARD_PORTS) break;
				vport[packet->port].write_port(packet->port, vport[packet->port].read_port(packet->port) | packet->pinmask);
				break;
			case udpIOEnableAndDisable:
				if (packet->port > IO_HARD_PORTS) break;
				vport[packet->port].write_port(packet->port, packet->pinmask);
				break;
			case udpIOGetAllPortPins:
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
				break;
			case udpIORegisterClientForPortChanges:
				// look for a not used entry in the array
				for (;i<ALLOWED_CLIENTS;++i) {
					if(uip_ipaddr_cmp(&(clients[i].address), nullip)) // found empty entry
						break;
				}
				// override last array entry if no free entry available
				if (i==ALLOWED_CLIENTS) i = ALLOWED_CLIENTS-1;
				// create entry with ip adress of the calling client, the ioport to observe, the pinmask and the current port state
				uip_ipaddr_copy(&(clients[i].address), BUF->srcipaddr);
				clients[i].pinport = packet->port;
				clients[i].pinmask = packet->pinmask;
				clients[i].last_pin_state = vport[packet->port].read_port(packet->port) & clients[i].pinmask;
				break;
			case udpIOUnRegisterClientForPortChanges:
				for (i=0;i<ALLOWED_CLIENTS;++i) {
					if(uip_ipaddr_cmp(&(clients[i].address), &(BUF->srcipaddr))) // found address of calling client in clients: remove it now
						uip_ipaddr(&(clients[i].address), 0,0,0,0);
				}
				break;
			default:
			break;
		};
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
