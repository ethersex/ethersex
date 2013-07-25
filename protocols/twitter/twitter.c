/*
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
#include "twitter.h"

static char *twitter_tmp_buf;

static const char PROGMEM twitter_header[] =
    "POST " CONF_TWITTER_API "/statuses/update.xml HTTP/1.1\n"
    "Authorization: Basic " CONF_TWITTER_AUTH "\n"
    "Host: " CONF_TWITTER_SERVICE "\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: ";

static const char PROGMEM twitter_body[] =
    "\n\nstatus=";


static void
twitter_net_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	TWDEBUG ("connection aborted\n");
        if (twitter_tmp_buf) {
          free(twitter_tmp_buf);
          twitter_tmp_buf = NULL;
        }
        return;
    }

    if (uip_closed()) {
	TWDEBUG ("connection closed\n");
        if (twitter_tmp_buf) {
          free(twitter_tmp_buf);
          twitter_tmp_buf = NULL;
        }
        return;
    }


    if (uip_connected() || uip_rexmit()) {
	TWDEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;
        p += sprintf_P(p, twitter_header);
        p += sprintf(p, "%d", strlen(twitter_tmp_buf) + 7);
        p += sprintf_P(p, twitter_body);
        p += sprintf(p, "%s", twitter_tmp_buf);
        uip_udp_send(p - (char *)uip_appdata);
        TWDEBUG("send %d bytes\n", p - (char *)uip_appdata);
    }

    if (uip_acked()) {
      uip_close();
    }

}

static void
twitter_dns_query_cb(char *name, uip_ipaddr_t *ipaddr) {
  TWDEBUG("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), twitter_net_main)) {
    if (twitter_tmp_buf) {
      free(twitter_tmp_buf);
      twitter_tmp_buf = NULL;
    }
  }

}

uint8_t
twitter_send(char *status)
{
  /* Transmission taking action */
  if (twitter_tmp_buf) return 0;

  uint8_t len = strlen(status);
  if (len > 140) {
    TWDEBUG("message too long: cropping");
    len = 140;
  }

  twitter_tmp_buf = malloc(140);
  if (!twitter_tmp_buf) return 0;

  memcpy(twitter_tmp_buf, status, len);
  twitter_tmp_buf[len] = 0;

  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_TWITTER_SERVICE))) {
    resolv_query(CONF_TWITTER_SERVICE, twitter_dns_query_cb);
  } else {
    twitter_dns_query_cb(NULL, ipaddr);
  }
  return 1;
}
