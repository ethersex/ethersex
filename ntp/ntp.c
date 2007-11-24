/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../net/ntp_net.h"
#include "../dns/resolv.h"
#include "ntp.h"

#ifdef NTP_SUPPORT

static struct uip_udp_conn *ntp_conn = NULL;
static void ntp_dns_query_cb(char *name, uip_ipaddr_t *ipaddr);
static void send_ntp_packet(void);
static uint32_t timestamp;
static uint16_t ntp_timer = 1;


void
ntp_init()
{
  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(NTP_SERVER))) 
    resolv_query(NTP_SERVER, ntp_dns_query_cb);
  else {
    if(ntp_conn != NULL) {
      uip_udp_remove(ntp_conn);
    }
    ntp_conn = uip_udp_new(ipaddr, HTONS(NTP_PORT), ntp_net_main);
  }
  ntp_timer = 1;
}

static void
ntp_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
  if(ntp_conn != NULL) {
    uip_udp_remove(ntp_conn);
  }
  ntp_conn = uip_udp_new(ipaddr, HTONS(NTP_PORT), ntp_net_main);
}

void 
ntp_periodic(void) 
{
  if (ntp_timer) 
    send_ntp_packet();
}

static void 
send_ntp_packet(void)
{
  struct ntp_packet pkt = { 0 };

  pkt.li_vn_mode = 0xe3; /* Clock not synchronized, Version 4, Client Mode */
  pkt.ppoll = 12; /* About an hour */
  pkt.precision = 0xfa; /* 0.015625 seconds */
  pkt.rootdelay = HTONL(0x10000); /* 1 second */
  pkt.rootdispersion = HTONL(0x10000); /* 1 second */
  
  uip_send(&pkt, sizeof(struct ntp_packet));
}

void
ntp_newdata(void)
{
  struct ntp_packet *pkt = uip_appdata;
  /* We must save an unix timestamp */
  timestamp = NTOHL(pkt->rec.seconds) - 2208988800;

  ntp_timer = 0;
}

uint32_t
get_time(void)
{
  return timestamp;
}

#endif
