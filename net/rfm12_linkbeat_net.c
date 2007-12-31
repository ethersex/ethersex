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

#include <avr/pgmspace.h>

#include "rfm12_linkbeat_net.h"
#include "udp_echo_net.h"
#include "../uip/uip.h"
#include "../config.h"

#if defined(RFM12_LINKBEAT_NET_SUPPORT) \
  && defined(RFM12_SUPPORT) && defined(ENC28J60_SUPPORT) 

const char PROGMEM linkbeat_msg[] = "e'sex rfm12 is cool.";
#define LINKBEAT_MSG_LEN 20

uint16_t linkbeat_sent;
uint16_t linkbeat_recv;

void
rfm12_linkbeat_net_init (void)
{
    uip_ipaddr_t ip;
    //CONF_RFM12_IP;
    uip_ip6addr(ip,0x2001,0x4b88,0x10e4,0x0,0x23,0x42,0x0,0x1);

    uip_udp_new (&ip, HTONS(ECHO_PORT), rfm12_linkbeat_net_main);
}


void
rfm12_linkbeat_net_main(void)
{
    if (uip_poll ()) {
	static uint8_t counter = 0;

	if ((counter ++) & 31)
	    return;		/* only send every 6 secs, roughly. */

	memcpy_P (uip_appdata, linkbeat_msg, LINKBEAT_MSG_LEN);
	uip_send (uip_appdata, LINKBEAT_MSG_LEN);

	linkbeat_sent ++;
    }

    else if (uip_newdata ()) {
	if (uip_len != LINKBEAT_MSG_LEN
	    || memcmp_P (uip_appdata, linkbeat_msg, LINKBEAT_MSG_LEN))
	    return;		/* failed. */

	linkbeat_recv ++;
    }
}

#endif /* RFM12_LINKBEAT_NET_SUPPORT */
