/*
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
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

// httplog.c
//
// based heavily on twitter.c
// purpose:
// providing httplog() which can be used to log an arbitrary string against a web server
// optionally including the current unix time stamp and a unique machine identifier (uuid)
// thus providing a similar facility as syslog

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#include "httplog.h"
#include "services/clock/clock.h"
#include <util/delay.h>

static char *httplog_tmp_buf;

// first string is the GET part including the path
static const char PROGMEM get_string_head[] =
    "GET " CONF_HTTPLOG_PATH "?";
// next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_HTTPLOG_INCLUDE_UUID
static const char PROGMEM uuid_string[] =
	"uuid=" CONF_HTTPLOG_UUID "&";
#endif
// the - optional - unix time stamp
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
static const char PROGMEM time_string[] =
	"time=";
#endif
// and the http footer including the http protocol version and the server name
static const char PROGMEM get_string_foot[] =
    " HTTP/1.1\n"
    "Host: " CONF_HTTPLOG_SERVICE "\n\n\n";


static void
httplog_net_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	HTTPLOG_DEBUG ("connection aborted\n");
        if (httplog_tmp_buf) {
          free(httplog_tmp_buf);
          httplog_tmp_buf = NULL;
        }
        return;
    }

    if (uip_closed()) {
	HTTPLOG_DEBUG ("connection closed\n");
        if (httplog_tmp_buf) {
          free(httplog_tmp_buf);
          httplog_tmp_buf = NULL;
        }
        return;
    }


    if (uip_connected() || uip_rexmit()) {
	HTTPLOG_DEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;
        p += sprintf_P(p, get_string_head);
#ifdef CONF_HTTPLOG_INCLUDE_UUID
        p += sprintf_P(p, uuid_string);
#endif
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
        p += sprintf_P(p, time_string);
 		p += sprintf(p, "%lu&", clock_get_time());
#endif
        p += sprintf(p, httplog_tmp_buf);
        p += sprintf_P(p, get_string_foot);
        uip_udp_send(p - (char *)uip_appdata);
        HTTPLOG_DEBUG("send %d bytes\n", p - (char *)uip_appdata);
    }

    if (uip_acked()) {
      uip_close();
    }

}

static void
httplog_dns_query_cb(char *name, uip_ipaddr_t *ipaddr) {
  HTTPLOG_DEBUG("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), httplog_net_main)) {
    if (httplog_tmp_buf) {
      free(httplog_tmp_buf);
      httplog_tmp_buf = NULL;
    }
  }

}

uint8_t
httplog(char status[140] )
{
  /* Transmission taking action */
  if (httplog_tmp_buf) return 0;

  uint8_t len = strlen(status);
  if (len > 140) {
    HTTPLOG_DEBUG("message too long: cropping");
    len = 140;
  }

  httplog_tmp_buf = malloc(140);
  if (!httplog_tmp_buf) return 0;

  memcpy(httplog_tmp_buf, status, len);
  httplog_tmp_buf[len] = 0;

  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_HTTPLOG_SERVICE))) {
    resolv_query(CONF_HTTPLOG_SERVICE, httplog_dns_query_cb);
  } else {
    httplog_dns_query_cb(NULL, ipaddr);
  }
  return 1;
}
