/*
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
 */

#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/dns/resolv.h"
#include "services/clock/clock.h"
#include "ntp.h"
#include "ntpd_net.h"
#include "core/debug.h"
#include "config.h"

static uip_udp_conn_t *ntp_conn = NULL;
#ifdef DNS_SUPPORT
static uint8_t ntp_tries = 0;
#endif

#ifdef DNS_SUPPORT
void
ntp_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
  ntp_conf(ipaddr);

#ifdef DEBUG_NTP
    debug_printf("NTP: query connected\n");
#endif
}
#endif

void
ntp_conf(uip_ipaddr_t *ntpserver)
{
  if (ntp_conn != NULL)
    uip_udp_remove(ntp_conn);
  ntp_conn = uip_udp_new(ntpserver, HTONS(NTP_PORT), ntp_newdata);
}

uip_ipaddr_t *
ntp_getserver(void)
{
  uip_ipaddr_t *ntpaddr = NULL;
  
  if (ntp_conn != NULL)
    ntpaddr = (uip_ipaddr_t *) ntp_conn->ripaddr;

  return ntpaddr;
}

void
ntp_init()
{
#ifdef DNS_SUPPORT
  ntp_tries = 0; // reset try counter
  uip_ipaddr_t *ipaddr;
  if (ntp_conn != NULL || !(ipaddr = resolv_lookup(NTP_SERVER)))
    resolv_query(NTP_SERVER, ntp_dns_query_cb);
  else
    ntp_conf(ipaddr);

#else /* ! DNS_SUPPORT */
  uip_ipaddr_t ip;
  set_NTP_SERVER_IP(&ip);

  ntp_conf(&ip);
#endif
}


void
ntp_send_packet(void)
{
#ifdef DNS_SUPPORT
  if (++ntp_tries >= 5) {
 #ifdef DEBUG_NTP
    debug_printf("NTP ntp_send_packet: re-init after %d unsuccessful tries\n", ntp_tries);
 #endif
    ntp_init();
    return;
  }
#endif

  if (ntp_conn == NULL || ntp_conn->ripaddr == NULL) {
#ifdef DEBUG_NTP
    debug_printf("NTP ntp_send_packet: skip send, ntp not initialized\n");
#endif
    return;
  } 
 
  /* LLH len defined in UIP depending on stacks (i.e. 14 for ethernet frame),
  may be already suitable for tunneling! */
  struct ntp_packet *pkt = (void *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];

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
#ifdef DEBUG_NTP
  debug_printf("NTP: send packet\n");
#endif
  router_output();
  uip_slen = 0;
}

void
ntp_newdata(void)
{
  if (!uip_newdata ()) return;

  uint32_t ntp_timestamp;
  struct ntp_packet *pkt = uip_appdata;
  /* We must save a unix timestamp */
  ntp_timestamp = NTOHL(pkt->rec.seconds) - JAN_1970;

#ifdef DEBUG_NTP
    debug_printf("NTP: Set new time: %lu\n",ntp_timestamp);
#endif
  clock_set_time(ntp_timestamp);
  set_dcf_count(0);
  set_ntp_count(1);
#ifdef NTPD_SUPPORT
  ntp_setstratum(pkt->stratum);
#endif

#ifdef DNS_SUPPORT
  ntp_tries = 0; // reset try counter
#endif
}

/*
  -- Ethersex META --
  header(services/ntp/ntp.h)
  net_init(ntp_init)
*/
