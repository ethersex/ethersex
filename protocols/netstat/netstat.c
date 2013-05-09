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
#include "protocols/dns/resolv.h"
#include "netstat.h"




static const char PROGMEM netstat_header[] =
    "POST " CONF_NETSTAT_API "update.php HTTP/1.1\n"
    "Host: " CONF_NETSTAT_SERVICE "\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: ";

static const char PROGMEM netstat_mac[] =
    "\n\nmac=";


static void
netstat_net_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	NETSTATDEBUG ("connection aborted\n");
        return;
    }

    if (uip_closed()) {
	NETSTATDEBUG ("connection closed\n");
        return;
    }

    if (uip_connected() || uip_rexmit()) {
	NETSTATDEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;
        p += sprintf_P(p, netstat_header);
        p += sprintf(p, "%d", 4 + 17); // -> mac=xx:xx:xx:xx:xx:xx
        p += sprintf_P(p, netstat_mac);
        p += sprintf(p, "%02x:%02x:%02x:%02x:%02x:%02x",
                        uip_ethaddr.addr[0],
                        uip_ethaddr.addr[1],
                        uip_ethaddr.addr[2],
                        uip_ethaddr.addr[3],
                        uip_ethaddr.addr[4],
                        uip_ethaddr.addr[5]
                        );
        uip_udp_send(p - (char *)uip_appdata);
        NETSTATDEBUG("send %d bytes\n", p - (char *)uip_appdata);
    }

    if (uip_acked()) {
      NETSTATDEBUG("ACK\n");
      uip_close();
    } else {
      NETSTATDEBUG("NACK\n");
    }

}

#ifdef DNS_SUPPORT
static void
netstat_dns_query_cb(char *name, uip_ipaddr_t *ipaddr) {
  NETSTATDEBUG("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), netstat_net_main)) {
  }

}
#endif  /* DNS_SUPPORT */

uint8_t
netstat_send()
{
  NETSTATDEBUG ("send\n");
#ifdef DNS_SUPPORT
  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_NETSTAT_SERVICE))) {
    resolv_query(CONF_NETSTAT_SERVICE, netstat_dns_query_cb);
  } else {
    netstat_dns_query_cb(NULL, ipaddr);
  }
#else
  uip_ipaddr_t ipaddr;
  set_CONF_NETSTAT_SERVICE_IP(&ipaddr);
  if (! uip_connect(&ipaddr, HTONS(80), netstat_net_main))
  {
  NETSTATDEBUG ("failed\n");
  }
#endif
  return 1;
}

/*
  -- Ethersex META --
  header(protocols/netstat/netstat.h)
  net_init(netstat_send)
*/
