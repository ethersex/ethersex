/*
 * Copyright (c) 2007 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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


enum DynDnsStates
{
  DYNDNS_HOSTNAME,
  DYNDNS_IP,
  DYNDNS_USERNAME,
  DYNDNS_PASSWORD,
  DYNDNS_READY,
  DYNDNS_CANCEL,
};

static void dyndns_query_cb(char *name, uip_ipaddr_t * ipaddr);
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
  for (i = 0; i < UIP_CONNS; i++)
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
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
    uip_connect(ipaddr, HTONS(80), dyndns_net_main);
#else
    dyndns_conn = uip_udp_connect(ipaddr, HTONS(17569), dyndns_net_main);
#endif
#else
  uip_ipaddr_t ipaddr;
  // dyn.metafnord.de
#ifdef IPV6_SUPPORT
  uip_ip6addr(&ipaddr, 0x2001, 0x6f8, 0x1209, 0x0, 0x0, 0x0, 0x0, 0x2);
#else
  uip_ipaddr(&ipaddr, 78, 47, 210, 243);
#endif

#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  uip_conn_t *conn = uip_connect(&ipaddr, HTONS(80), dyndns_net_main);
  if (conn)
    conn->appstate.dyndns.state = DYNDNS_HOSTNAME;
#else
  dyndns_conn = uip_udp_new(&ipaddr, HTONS(17569), dyndns_net_main);
#endif /* TCP and not TEENSY */

#endif
}

static void
dyndns_query_cb(char *name, uip_ipaddr_t * ipaddr)
{
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  uip_conn_t *conn = uip_connect(ipaddr, HTONS(80), dyndns_net_main);
  if (conn)
    conn->appstate.dyndns.state = DYNDNS_HOSTNAME;
#else
  dyndns_conn = uip_udp_new(ipaddr, HTONS(17569), dyndns_net_main);
#endif /* TCP and not TEENSY */
}

/* Helper functions */
#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'a'))
#if !defined(TCP_SUPPORT) || defined(TEENSY_SUPPORT)
#ifdef IPV6_SUPPORT
static char *
uint16toa(char *p, uint16_t i)
{
  uint8_t x = 16;
  uint8_t tmp;
  char *begin = p;
  do
  {
    x -= 4;
    tmp = (i >> x) & 0x0F;
    if (tmp || p != begin || x == 0)
      *p++ = NIBBLE_TO_HEX(tmp);
  }
  while (x);
  return p;
}
#else
static char *
uint8toa(char *p, uint8_t i)
{
  uint8_t tmp;
  tmp = i / 100;
  if (tmp)
    *p++ = tmp + '0';
  i -= tmp * 100;
  tmp = i / 10;
  if (tmp)
    *p++ = tmp + '0';
  i -= tmp * 10;
  *p++ = i + '0';
  return p;
}
#endif
#endif

static void
dyndns_net_main(void)
{
#if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
  /* Close connection on ready an when cancel was requested */
  if (uip_conn->appstate.dyndns.state >= DYNDNS_READY)
  {
    uip_abort();
    return;
  }

  if (uip_acked())
  {
    uip_conn->appstate.dyndns.state++;
    if (uip_conn->appstate.dyndns.state == DYNDNS_READY)
      uip_close();
  }

  if (uip_rexmit() || uip_connected() || uip_acked())
  {
    uint8_t len;
    uip_ipaddr_t ipaddr;
#ifdef IPV6_SUPPORT
    uint16_t *ip6;
#else
    uint8_t *ip;
#endif

    switch (uip_conn->appstate.dyndns.state)
    {
      case DYNDNS_HOSTNAME:
        len = sprintf_P(uip_sappdata,
                        PSTR("GET /edit.cgi?name=" CONF_DYNDNS_HOSTNAME "&"));
        break;
      case DYNDNS_IP:
        uip_gethostaddr(&ipaddr);
#ifdef IPV6_SUPPORT
        ip6 = (uint16_t *) & ipaddr;
        len = sprintf_P(uip_sappdata,
                        PSTR("ip=%x%%3A%x%%3A%x%%3A%x%%3A%x%%3A%x"
                             "%%3A%x%%3A%x&"), HTONS(ip6[0]),
                        HTONS(ip6[1]), HTONS(ip6[2]), HTONS(ip6[3]),
                        HTONS(ip6[4]), HTONS(ip6[5]), HTONS(ip6[6]),
                        HTONS(ip6[7]));
#else
        ip = (uint8_t *) & ipaddr;
        len = sprintf_P(uip_sappdata,
                        PSTR("ip=%u.%u.%u.%u&"),
                        ip[0], ip[1], ip[2], ip[3]);
#endif
        break;
      case DYNDNS_USERNAME:
        len = sprintf_P(uip_sappdata,
                        PSTR("username=" CONF_DYNDNS_USERNAME "&"));
        break;
      case DYNDNS_PASSWORD:
        len = sprintf_P(uip_sappdata,
                        PSTR("password=" CONF_DYNDNS_PASSWORD
                             " HTTP/1.1\r\n"
                             "Host: dyn.metafnord.de\r\n\r\n"));
        break;
    }

    uip_send(uip_sappdata, len);
  }
#else /* TCP and not TEENSY_SUPPORT */
  if (uip_newdata() && dyndns_conn)
  {
    uip_udp_remove(dyndns_conn);
  }
  else if (uip_poll() && ((poll_counter++ % 5) == 0))
  {
    /* Don't try to update the dyndns entry forever */
    if (poll_counter > 55)
      uip_udp_remove(dyndns_conn);

    /* Send a dyndns packet */
    uint8_t len;
    char *p = uip_appdata;
    /* Form: 'username:password@hostname ip' */
    len = sizeof(CONF_DYNDNS_USERNAME ":"
                 CONF_DYNDNS_PASSWORD "@" CONF_DYNDNS_HOSTNAME " ") - 1;
    memcpy_P(p, PSTR(CONF_DYNDNS_USERNAME ":"
                     CONF_DYNDNS_PASSWORD "@" CONF_DYNDNS_HOSTNAME " "), len);
    p += len;
    len = 0;
    uint16_t *in16 = uip_hostaddr;
#ifdef IPV6_SUPPORT
    while (len++ < 8)
    {
      p = uint16toa(p, HTONS(*in16));
      *p++ = ':';
      in16++;
    }
#else
    while (len++ < 2)
    {
      uint8_t i = *in16, tmp;
      p = uint8toa(p, *in16);
      *p++ = '.';
      p = uint8toa(p, (*in16) >> 8);
      *p++ = '.';
      in16++;
    }
#endif
    p[-1] = '\n';
    uip_udp_send(p - (char *) uip_appdata);

  }
#endif /* TCP and not TEENSY_SUPPORT */
}

/*
  -- Ethersex META --
  state_header(services/dyndns/dyndns_state.h)
  state_tcp(`
#   if defined(TCP_SUPPORT) && !defined(TEENSY_SUPPORT)
       struct dyndns_connection_state_t dyndns;
#   endif
  ')
*/
