/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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


#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#include "core/debug.h"
#include "dyndns.h"


#ifdef DYNDNS_SUPPORT
static void dyndns_query_cb(char *name, uip_ipaddr_t *ipaddr);
static void dyndns_net_main(void);
#if !(defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT))
static uip_udp_conn_t *dyndns_conn = NULL;
static uint8_t poll_counter = 5;
#endif

void
dyndns_update(void)
{
 uint8_t i;
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  /* Request to close all other dyndns connections */
  for (i = 0; i < UIP_CONNS; i ++)
    if (uip_conns[i].callback == dyndns_net_main)
      uip_conns[i].appstate.dyndns.state = DYNDNS_CANCEL;
#else
  /* No TCP_SUPPORT */
  if (dyndns_conn)
    uip_udp_remove(dyndns_conn);
#endif

#ifdef DNS_SUPPORT
  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup("dyn.metafnord.de")))
    resolv_query("dyn.metafnord.de", dyndns_query_cb);
  else
#   if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
    uip_connect(ipaddr, HTONS(80), dyndns_net_main);
#   else
    dyndns_conn = uip_udp_connect(ipaddr, HTONS(17569), dyndns_net_main);
#   endif
#else
  uip_ipaddr_t ipaddr;
  // dyn.metafnord.de
#ifdef IPV6_SUPPORT
  uip_ip6addr(&ipaddr, 0x2001, 0x6f8, 0x1209, 0x0, 0x0, 0x0, 0x0, 0x2);
#else
  uip_ipaddr(&ipaddr, 78, 47, 210, 243);
#endif

#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  uip_conn_t *conn = uip_connect (&ipaddr, HTONS (80), dyndns_net_main);
  if (conn)
    conn->appstate.dyndns.state = DYNDNS_HOSTNAME;
#else
    dyndns_conn = uip_udp_new(&ipaddr, HTONS(17569), dyndns_net_main);
#endif /* TCP and not TEENSY */

#endif
}

static void
dyndns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  uip_conn_t *conn = uip_connect (ipaddr, HTONS (80), dyndns_net_main);
  if (conn)
    conn->appstate.dyndns.state = DYNDNS_HOSTNAME;
#else
    dyndns_conn = uip_udp_new(ipaddr, HTONS(17569), dyndns_net_main);
#endif /* TCP and not TEENSY */
}
/* Helper functions */
#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'a'))
#if !defined(TCP_SUPPORT) || defined(TEENSY_SUPPORT)
#  ifdef IPV6_SUPPORT
static char *
uint16toa(char *p, uint16_t i) {
  uint8_t x = 16;
  uint8_t tmp;
  char *begin = p;
  do {
    x -= 4;
    tmp = (i >> x) & 0x0F;
    if (tmp || p != begin || x == 0)
      *p++ = NIBBLE_TO_HEX(tmp);
  } while (x);
  return p;
}
#  else
static char *
uint8toa(char *p, uint8_t i) {
  uint8_t tmp;
  tmp = i / 100;
  if (tmp) *p++ = tmp + '0';
  i -= tmp * 100;
  tmp = i / 10;
  if (tmp) *p++ = tmp + '0';
  i -= tmp * 10;
  *p++ = i + '0';
  return p;
}
#  endif
#endif

static void
dyndns_net_main(void)
{
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  /* Close connection on ready an when cancel was requested */
  if (uip_conn->appstate.dyndns.state >= DYNDNS_READY) {
    uip_abort ();
    return;
  }

  if(uip_acked()) {
    uip_conn->appstate.dyndns.state ++;
    if (uip_conn->appstate.dyndns.state == DYNDNS_READY)
      uip_close();
  }

  if(uip_rexmit() ||  uip_connected() || uip_acked())
  {
    char *to_be_sent;
    uint8_t length;
    uip_ipaddr_t ipaddr;
#ifdef IPV6_SUPPORT
    uint16_t *ip6;
#else
    uint8_t *ip;
#endif

    switch(uip_conn->appstate.dyndns.state) {
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
  }
#else /* TCP and not TEENSY_SUPPORT */
  if (uip_newdata() && dyndns_conn) {
    uip_udp_remove(dyndns_conn);
  } else if (uip_poll() && ((poll_counter++ % 5) == 0)) {
    /* Don't try to update the dyndns entry forever */
    if (poll_counter > 55)
      uip_udp_remove(dyndns_conn);

    /* Send a dyndns packet */
    uint8_t len;
    char *p = uip_appdata;
    /* Form: 'username:password@hostname ip' */
    len = strlen_P(PSTR(CONF_DYNDNS_USERNAME ":"
                        CONF_DYNDNS_PASSWORD "@"
                        CONF_DYNDNS_HOSTNAME " "));
    memcpy_P(p, PSTR(CONF_DYNDNS_USERNAME ":"
                     CONF_DYNDNS_PASSWORD "@"
                     CONF_DYNDNS_HOSTNAME " "),
             len);
    p += len;
    len = 0;
    uint16_t *in16 = uip_hostaddr;
#ifdef IPV6_SUPPORT
    while (len++ < 8) {
      p = uint16toa(p, HTONS(*in16));
      *p++ = ':';
      in16++;
    }
#else
    while (len++ < 2) {
        uint8_t i = *in16, tmp;
        p = uint8toa(p, *in16);
        *p++ = '.';
        p = uint8toa(p, (*in16) >> 8);
        *p++ = '.';
        in16++;
    }
#endif
    p[-1] = '\n';
    uip_udp_send(p - (char *)uip_appdata);

  }
#endif /* TCP and not TEENSY_SUPPORT */
}

#endif

/*
  -- Ethersex META --
  state_header(services/dyndns/dyndns_state.h)
  state_tcp(`
#   if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
       struct dyndns_connection_state_t dyndns;
#   endif
')
*/

