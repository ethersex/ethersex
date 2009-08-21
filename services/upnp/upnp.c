/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "upnp.h"


static const char PROGMEM upnp_header[] =
	"NOTIFY * HTTP/1.1\r\n"
	"Host:" CONF_UPNP_MULTICAST_IP ":1900\r\n"
	"NT:upnp:rootdevice\r\n"
	"NTS:ssdp:alive\r\n"
	"Location:http://" CONF_ENC_IP ":80" CONF_UPNP_SCHEME_FILENAME "\r\n"
	"USN:uuid:00000000-0000-0001-0001-0001e3d766e5::upnp:rootdevice\r\n"
	"SERVER: " CONF_HOSTNAME " UPnP/1.0\r\n"
	"Cache-Control:max-age=60\r\n\r\n";

void
upnp_send (void)
{
  UPNPDEBUG ("send\n");

  uip_udp_conn_t upnp_conn;
  set_CONF_UPNP_MULTICAST_IP(&upnp_conn.ripaddr);  
  upnp_conn.rport = HTONS(1900);
  upnp_conn.lport = HTONS(1900);

  uip_udp_conn = &upnp_conn;

  strcpy_P (&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], upnp_header);

  uip_slen = sizeof (upnp_header) - 1;
  uip_process (UIP_UDP_SEND_CONN);
  router_output ();

  uip_slen = 0;
}

/*
  -- Ethersex META --
  header(services/upnp/upnp.h)
  timer(100,upnp_send())
*/
