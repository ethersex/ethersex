/* 
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
#include "core/debug.h"
#include "protocols/uip/uip.h"

#include "rfm12.h"
#include "rfm12_raw_net.h"

uip_udp_conn_t *rfm12_raw_conn;

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
rfm12_raw_net_init(void)
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  rfm12_raw_conn = uip_udp_new(&ip, 0, rfm12_raw_net_main);
  if (!rfm12_raw_conn)
    return;

  uip_udp_bind(rfm12_raw_conn, HTONS(RFM12_RAW_UDP_PORT));
}

void
rfm12_raw_net_main(void)
{
  if (uip_newdata())
  {
    /* 600ms timeout */
    uip_udp_conn->appstate.rfm12_raw.timeout = 3;

    uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
    uip_udp_conn->rport = BUF->srcport;

    memmove(rfm12_data, uip_appdata, uip_len);

    rfm12_txstart(uip_len);
    return;
  }

  if (!uip_udp_conn->appstate.rfm12_raw.timeout)
    return;

  if (--uip_udp_conn->appstate.rfm12_raw.timeout)
    return;                     /* timeout not yet over. */

  uip_ipaddr_copy(uip_udp_conn->ripaddr, all_ones_addr);
  uip_udp_conn->rport = 0;
}

/*
  -- Ethersex META --
  header(hardware/radio/rfm12/rfm12_raw_net.h)
  net_init(rfm12_raw_net_init)
  state_header(hardware/radio/rfm12/rfm12_raw_state.h)
  state_udp(struct rfm12_raw_connection_state_t rfm12_raw)
*/
