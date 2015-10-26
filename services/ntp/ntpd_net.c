/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Hans Baechle <hans.baechle@gmx.net>
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

#include <avr/io.h>

#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "services/clock/clock.h"
#include "ntp.h"
#include "config.h"
#include "ntpd_net.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))
static uint8_t ntp_stratum = 0;

void
ntpd_net_init(void)
{
  uip_udp_conn_t *conn;
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  if(! (conn = uip_udp_new(&ip, 0, ntpd_net_main)))
    return; /* Couldn't bind socket */

  uip_udp_bind(conn, HTONS(NTPD_PORT));
}

void
ntpd_net_main(void)
{
  if (uip_newdata()) {
    struct ntp_packet *pkt = uip_appdata;
    uint32_t last_sync = clock_last_sync();
    uint32_t current_timestamp = 0;
    uint8_t  current_fraction  = 0;
    /*We try to minimize the time between the two calls*/
    current_timestamp = clock_get_time();
    current_fraction = clock_get_ticks();
    /* Set our time to the packet */
    pkt->rec.seconds = HTONL(current_timestamp + JAN_1970);
    /*  the clock timer is 8-bit and gives us steps of 0.0039 seconds,
	the fraction part of the ntp protocol has an accuracy of 32 bit, therefore we shift
	with 24 (not optimized: counter_fraction/255.00*4294967296) */
    pkt->rec.fraction = HTONL((uint32_t)current_fraction << 24);

    /* set the update time (reference clock) */
    pkt->reftime.seconds = HTONL(last_sync + JAN_1970);
    pkt->reftime.fraction = HTONL((uint32_t)clock_last_sync_tick() << 24);

    /* We are an server and there is no error warning */
    pkt->li_vn_mode = 0x24;

    /* copy the old transmit timestamp */
    pkt->org.seconds = pkt->xmt.seconds;
    pkt->org.fraction = pkt->xmt.fraction;

    /* Set what type of clock we are */
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
    int stratum = ntp_getstratum();

    pkt->stratum = (last_sync > 0) ? stratum + 1 : 0;
    
    if (stratum == 0)
	{
	pkt->refid = 0x61464344;	/* DCFa in Network byte order */
	pkt->precision = 0xEC;
	pkt->rootdispersion = 0x90000000;
	}
    else {
#ifdef NTP_SUPPORT
        if (sizeof(uip_ipaddr_t) == 4)
          {
            uip_ipaddr_copy((uip_ipaddr_t *) &pkt->refid, ntp_getserver());
	    pkt->precision = 0xEC;
	    pkt->rootdispersion = 0x95000000;
	  }
        else
#endif /* NTP_SUPPORT */
	  {
            pkt->refid = 0x76677976;	/* some virtual identifer */
	    pkt->precision = 0xEC;
	    pkt->rootdispersion = 0xA5000000;
	  }
    }
#endif /* NTP_SUPPORT || DCF_SUPPORT */
    /*We try to minimize the time between the two calls*/
    current_timestamp=clock_get_time();
    current_fraction=TIMER_8_AS_1_COUNTER_CURRENT;
    /* copy the time also to the transmit time */
    pkt->xmt.seconds = HTONL(current_timestamp + JAN_1970);
    pkt->xmt.fraction = HTONL((uint32_t)current_fraction << 24);

    uip_udp_send(sizeof(struct ntp_packet));

    /* Send the packet */
    uip_udp_conn_t conn;
    uip_ipaddr_copy(conn.ripaddr, BUF->srcipaddr);
    conn.rport = BUF->srcport;
    conn.lport = HTONS(NTPD_PORT);

    uip_udp_conn = &conn;

    /* Send immediately */
    uip_process(UIP_UDP_SEND_CONN);
    router_output();

    uip_slen = 0;
  }
}

uint8_t
ntp_getstratum(void)
{
  return ntp_stratum;
}

void
ntp_setstratum(uint8_t stratum)
{
  ntp_stratum = stratum;
}


/*
  -- Ethersex META --
  header(services/ntp/ntpd_net.h)
  net_init(ntpd_net_init)
*/
