/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
 }}} */


#define MAX_BUFFER_LEN 100

#include "../uip/uip.h"
#include "../dns/resolv.h"
#include "../debug.h"
#include "dyndns.h"

#include "../config.h"

#ifdef DYNDNS_SUPPORT
static void dyndns_query_cb(char *name, uip_ipaddr_t *ipaddr);
static void dyndns_net_main(void);


enum DynDnsStates {
  DYNDNS_HOSTNAME,
  DYNDNS_IP,
  DYNDNS_USERNAME,
  DYNDNS_PASSWORD,
  DYNDNS_READY,
};

static uint8_t state = DYNDNS_HOSTNAME;

void
dyndns_update(void)
{
  state = DYNDNS_HOSTNAME;
#ifdef DNS_SUPPORT
  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup("dyn.metafnord.de"))) 
    resolv_query("dyn.metafnord.de", dyndns_query_cb);
  else
    uip_connect(ipaddr, HTONS(80), dyndns_net_main);
#else
  uip_ipaddr_t ipaddr;
  // dyn.metafnord.de
#ifdef IPV6_SUPPORT
  uip_ip6addr(&ipaddr, 0x2001, 0x6f8, 0x1209, 0x0, 0x216, 0x3eff, 0xfe03, 0xa8);
#else
  uip_ipaddr(&ipaddr, 78, 47, 210, 243);
#endif
  uip_connect(&ipaddr, HTONS(80), dyndns_net_main);
#endif
}

static void
dyndns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
  uip_connect(ipaddr, HTONS(80), dyndns_net_main);
}

static void
dyndns_net_main(void) 
{
  if(uip_acked()) {
    state++;
    if (state == DYNDNS_READY)
      uip_close();
  }

  if(uip_rexmit() ||  uip_connected() || uip_acked()) 
  {
    char *to_be_sent;
    uint8_t length;
    uip_ipaddr_t ipaddr;
    uint8_t *ip;
    uint16_t *ip6;

    switch(state) {
    case  DYNDNS_HOSTNAME:
      to_be_sent = __builtin_alloca(strlen_P(PSTR("GET /edit.cgi?name=%S&"))
        + strlen(CONF_DYNDNS_HOSTNAME));
      if(!to_be_sent) {
        debug_printf("ddns: out of memory\n");
        return;
      }
      length = sprintf_P(to_be_sent, PSTR("GET /edit.cgi?name=%S&"),
                         PSTR(CONF_DYNDNS_HOSTNAME));
      break;
    case DYNDNS_IP:
#ifdef IPV6_SUPPORT
      to_be_sent = __builtin_alloca(strlen_P(PSTR("ip=%x%%3A%x%%3A%x%%3A%x%%3A"
                                                  "%x%%3A%x%%3A%x%%3A%x&")) + 17);
#else
      to_be_sent = __builtin_alloca(strlen_P(PSTR("ip=%u.%u.%u.%u&")) + 4);
#endif
      if(!to_be_sent) {
        debug_printf("ddns: out of memory\n");
        return;
      }
    

      uip_gethostaddr(&ipaddr);
#ifdef IPV6_SUPPORT
      ip6 = (uint16_t *) &ipaddr;
      length = sprintf_P(to_be_sent, PSTR("ip=%x%%3A%x%%3A%x%%3A%x%%3A%x%%3A%x"
                                          "%%3A%x%%3A%x&"),
                         HTONS(ip6[0]), HTONS(ip6[1]), HTONS(ip6[2]), 
                         HTONS(ip6[3]), HTONS(ip6[4]), HTONS(ip6[5]),
                         HTONS(ip6[6]), HTONS(ip6[7]));
#else
      ip = (uint8_t *) &ipaddr;
      length = sprintf_P(to_be_sent, PSTR("ip=%u.%u.%u.%u&"), ip[0], ip[1], 
                         ip[2], ip[3]);
#endif
      break;
    case DYNDNS_USERNAME:
      to_be_sent = __builtin_alloca(strlen_P(PSTR("username=%S&"))
        + strlen(CONF_DYNDNS_USERNAME));
      if(!to_be_sent) {
        debug_printf("ddns: out of memory\n");
        return;
      }
      length = sprintf_P(to_be_sent, PSTR("username=%S&"),
                         PSTR(CONF_DYNDNS_USERNAME));
      break;
    case DYNDNS_PASSWORD:
      to_be_sent = 
        __builtin_alloca(strlen_P(PSTR("password=%S HTTP/1.1\r\nHost: "
                                       "dyn.metafnord.de\r\n\r\n"))
        + strlen(CONF_DYNDNS_PASSWORD));
      if(!to_be_sent) {
        debug_printf("ddns: out of memory\n");
        return;
      }
      length = 
        sprintf_P(to_be_sent, 
                  PSTR("password=%S HTTP/1.1\r\nHost: dyn.metafnord.de\r\n\r\n"),
                  PSTR(CONF_DYNDNS_PASSWORD));
      break;
      }

    uip_send(to_be_sent, length);
    debug_printf("%s\n", to_be_sent);
  }

}

#endif

