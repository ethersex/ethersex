/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include "protocols/uip/uip.h"
#include "config.h"
#include "core/debug.h"
#include "zbus.h"
#include "zbus_raw_net.h"

uip_udp_conn_t *zbus_raw_conn;

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
zbus_raw_net_init(void)
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  zbus_raw_conn = uip_udp_new(&ip, 0, zbus_raw_net_main);
  if(! zbus_raw_conn) return;

  uip_udp_bind (zbus_raw_conn, HTONS(ZBUS_RAW_UDP_PORT));
}



void
zbus_raw_net_main(void)
{
  if (uip_newdata ())
    {
      /* 600ms timeout */
      uip_udp_conn->appstate.zbus_raw.timeout = 3;

      uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
      uip_udp_conn->rport = BUF->srcport;

      memmove(zbus_buf, uip_appdata, uip_len);
      zbus_txstart(uip_len);
      return;
    }

  if (! uip_udp_conn->appstate.zbus_raw.timeout)
    return;

  if (-- uip_udp_conn->appstate.zbus_raw.timeout)
    return;			/* timeout not yet over. */

  uip_ipaddr_copy(uip_udp_conn->ripaddr, all_ones_addr);
  uip_udp_conn->rport = 0;
}

/*
  -- Ethersex META --
  header(protocols/zbus/zbus_raw_net.h)
  net_init(zbus_raw_net_init)

  state_header(protocols/zbus/zbus_raw_state.h)
  state_udp(struct zbus_raw_connection_state_t zbus_raw)
*/
