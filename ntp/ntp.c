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

static struct uip_udp_conn *ntp_conn = NULL;
static void send_ntp_packet(void);
static uint32_t timestamp = 1;
static uint32_t ntp_timestamp = 0;
static uint16_t ntp_timer = 1;


#ifdef DNS_SUPPORT
static void
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
    resolv_query(NTP_SERVER, ntp_dns_query_cb);

  else
    ntp_conn = uip_udp_new(ipaddr, HTONS(NTP_PORT), ntp_net_main);

#else /* ! DNS_SUPPORT */
  uip_ipaddr_t ipaddr;
  NTP_IPADDR(&ipaddr);
  ntp_conn = uip_udp_new(&ipaddr, HTONS(NTP_PORT), ntp_net_main);
#endif
}


void
ntp_every_second(void) 
{
  if(ntp_timer) {
    if((-- ntp_timer) == 0)
      send_ntp_packet();
  }

  if(timestamp <= 50 && (timestamp % 5 == 0))
    send_ntp_packet();

  if(ntp_timestamp <= timestamp)
    timestamp ++;
}

static void 
send_ntp_packet(void)
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
  struct ntp_packet *pkt = uip_appdata;
  /* We must save an unix timestamp */
  ntp_timestamp = NTOHL(pkt->rec.seconds) - 2208988800;

  /* Allow the clock to jump forward, but never ever to go backward. */
  if (ntp_timestamp > timestamp)
    timestamp = ntp_timestamp;

  ntp_timer = 4096;
}

uint32_t
get_time(void)
{
  return timestamp;
}
