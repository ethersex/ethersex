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

static char *netstat_tmp_buf;

static const char PROGMEM netstat_header[] =
    "POST " CONF_NETSTAT_API "update.php HTTP/1.1\n"
    "Host: " CONF_NETSTAT_SERVICE "\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: ";

static const char PROGMEM netstat_body[] =
    "\n\nmac=" CONF_ENC_MAC;


static void
netstat_net_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	NETSTATDEBUG ("connection aborted\n");
        if (netstat_tmp_buf) {
          free(netstat_tmp_buf);
          netstat_tmp_buf = NULL;
        }
        return;
    }

    if (uip_closed()) {
	NETSTATDEBUG ("connection closed\n");
        if (netstat_tmp_buf) {
          free(netstat_tmp_buf);
          netstat_tmp_buf = NULL;
        }
        return;
    }


    if (uip_connected() || uip_rexmit()) {
	NETSTATDEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;
        p += sprintf_P(p, netstat_header);
        p += sprintf(p, "%d", strlen(netstat_tmp_buf) + 7);
        p += sprintf_P(p, netstat_body);
        p += sprintf(p, netstat_tmp_buf);
        uip_udp_send(p - (char *)uip_appdata);
        NETSTATDEBUG("send %d bytes\n", p - (char *)uip_appdata);
    }

    if (uip_acked()) {
      uip_close();
    }

}

static void
netstat_dns_query_cb(char *name, uip_ipaddr_t *ipaddr) {
  NETSTATDEBUG("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), netstat_net_main)) {
    if (netstat_tmp_buf) {
      free(netstat_tmp_buf);
      netstat_tmp_buf = NULL;
    }
  }

}

uint8_t
netstat_send(char *status)
{
  /* Transmission taking action */
  if (netstat_tmp_buf) return 0;

  uint8_t len = strlen(status);
  if (len > 140) {
    NETSTATDEBUG("message too long: cropping");
    len = 140;
  }

  netstat_tmp_buf = malloc(140);
  if (!netstat_tmp_buf) return 0;

  memcpy(netstat_tmp_buf, status, len);
  netstat_tmp_buf[len] = 0;

  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_NETSTAT_SERVICE))) {
    resolv_query(CONF_NETSTAT_SERVICE, netstat_dns_query_cb);
  } else {
    netstat_dns_query_cb(NULL, ipaddr);
  }
  return 1;
}

/*
  -- Ethersex META --
*/
