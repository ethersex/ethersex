/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "zbus.h"
#include "zbus_raw_net.h"


void
zbus_process(void)
{
  zbus_index_t recv_len = zbus_rxfinish();
  if (! recv_len)
    return;
  uip_len = recv_len;

#ifdef ROUTER_SUPPORT
#ifdef ZBUS_RAW_SUPPORT
  if (zbus_raw_conn->rport) {
    /* zbus raw capturing active, forward in udp/ip encapsulated form,
       thusly don't push to the stack. */
    uip_udp_conn = zbus_raw_conn;
    memmove(uip_appdata, zbus_buf, recv_len);
    uip_slen = recv_len;
    uip_process(UIP_UDP_SEND_CONN);
    router_output ();

    uip_buf_unlock ();
    zbus_rxstart ();
    return;
  }
#endif
  /* uip_input expects the number of bytes including the LLH. */
  uip_len = uip_len + ZBUS_BRIDGE_OFFSET;

#endif

  zbus_rxstart ();

  router_input (STACK_ZBUS);

  if (!uip_len) {
    uip_buf_unlock ();
    return;
  }

  /* send buffer out */
  router_output ();

  uip_len = 0;
}

/*
  -- Ethersex META --
  header(protocols/zbus/zbus.h)
  mainloop(zbus_process)
*/
