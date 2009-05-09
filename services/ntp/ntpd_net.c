/*
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

#include <avr/io.h>

#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "services/clock/clock.h"
#include "ntp.h"
#include "config.h"
#include "ntpd_net.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

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

    /* We are an server and there is no error warning */
    pkt->li_vn_mode = 0x24;

    /* copy the old transmit timestamp */
    pkt->org.seconds = pkt->xmt.seconds;
    pkt->org.fraction = pkt->xmt.fraction;

    /* Set our time to the packet */
    pkt->rec.seconds = HTONL(clock_get_time() + 2208988800);
    pkt->rec.fraction = HTONL(((uint32_t)TCNT2) << 7);

    /* copy the recieve time also to the transmit time */
    pkt->xmt.seconds = pkt->rec.seconds;
    pkt->xmt.fraction = pkt->rec.fraction;

    /* set the reference clock */
    pkt->reftime.seconds = HTONL(last_sync + 2208988800);

    /* Set what type of clock we are */
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
    int stratum = ntp_getstratum();

    pkt->stratum = (last_sync > 0) ? stratum + 1 : 0;
    
    if (stratum == 1)
	pkt->refid = 0x68677000;	/* DCF in Network byte order */
    else {
        if (sizeof(uip_ipaddr_t) == 4)
	    uip_ipaddr_copy((uip_ipaddr_t *) &pkt->refid, ntp_getserver());
	else
	    pkt->refid = 0x01020304;	/* some virtual identifer */
    }
#endif /* NTP_SUPPORT || DCF_SUPPORT */

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

/*
  -- Ethersex META --
  header(services/ntp/ntpd_net.h)
  net_init(ntpd_net_init)
*/
