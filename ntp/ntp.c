/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include "../clock/clock.h"
#include "ntp.h"

static uip_udp_conn_t *ntp_conn = NULL;

#if defined(DNS_SUPPORT) || defined(BOOTP_SUPPORT)
void
ntp_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
  if(ntp_conn != NULL) {
    uip_udp_remove(ntp_conn);
  }
  ntp_conn = uip_udp_new(ipaddr, HTONS(NTP_PORT), ntp_net_main);
}
#endif


void
ntp_init()
{
  if(ntp_conn != NULL)
    uip_udp_remove(ntp_conn);

#ifdef DNS_SUPPORT
  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(NTP_SERVER))) 
    resolv_query(NTP_SERVER, ntp_dns_query_cb, NULL);

  else
    ntp_conn = uip_udp_new(ipaddr, HTONS(NTP_PORT), ntp_net_main);

#else /* ! DNS_SUPPORT */
  uip_ipaddr_t ipaddr;
  NTP_IPADDR(&ipaddr);
  ntp_conn = uip_udp_new(&ipaddr, HTONS(NTP_PORT), ntp_net_main);
#endif
}


void 
ntp_send_packet(void)
{
  /* hardcode for LLH len of 14 bytes (i.e. ethernet frame),
     this is not suitable for tunneling! */
  struct ntp_packet *pkt = (void *) &uip_buf[14 + UIP_IPUDPH_LEN];

  uip_slen = sizeof(struct ntp_packet);
  memset(pkt, 0, uip_slen);

  pkt->li_vn_mode = 0xe3; /* Clock not synchronized, Version 4, Client Mode */
  pkt->ppoll = 12; /* About an hour */
  pkt->precision = 0xfa; /* 0.015625 seconds */
  pkt->rootdelay = HTONL(0x10000); /* 1 second */
  pkt->rootdispersion = HTONL(0x10000); /* 1 second */

  /* push the packet out ... */
  uip_udp_conn = ntp_conn;
  uip_process(UIP_UDP_SEND_CONN);
}

void
ntp_newdata(void)
{
  uint32_t ntp_timestamp;

  struct ntp_packet *pkt = uip_appdata;
  /* We must save an unix timestamp */
  ntp_timestamp = NTOHL(pkt->rec.seconds) - 2208988800;

  clock_set_time(ntp_timestamp);

}
