/* vim:fdm=marker et ai
 * {{{
 *
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
 }}} */

#include "../config.h"
#include "../uip/uip.h"
#include "../uip/uip_router.h"
#include "zbus.h"
#include "../net/zbus_raw_net.h"


void
zbus_process(void)
{
  struct zbus_ctx *recv = zbus_rxfinish();
  if (! (recv && recv->len))
    return;

#ifdef ROUTER_SUPPORT
  memcpy(uip_buf + ZBUS_BRIDGE_OFFSET, recv->data, recv->len);

  /* uip_input expects the number of bytes including the LLH. */
  uip_len = recv->len + ZBUS_BRIDGE_OFFSET;

#ifdef ZBUS_RAW_SUPPORT
  if (zbus_raw_conn->rport) {
    /* zbus raw capturing active, forward in udp/ip encapsulated form,
       thusly don't push to the stack. */
    uip_udp_conn = zbus_raw_conn;
    uip_stack_set_active (STACK_ENC);
    memmove (uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN,
	     uip_buf + ZBUS_BRIDGE_OFFSET, recv->len);
    uip_slen = recv->len;
    uip_process(UIP_UDP_SEND_CONN);
    router_output ();

    recv->len = 0;		/* receive buffer may be overriden
                                   from now on. */
    
    zbus_rxstart ();
    return;
  }
#endif

#else  /* not ENC28J60_SUPPORT */
  /* We don't need to copy from recv->data, since ZBus already shares
     the input buffer.

     memcpy(uip_buf, recv->data, recv->len); */
  uip_len = recv->len;
  
#endif

  recv->len = 0;		/* receive buffer may be overriden
				   from now on. */

  router_input (STACK_ZBUS);

  if (!uip_len) {
    zbus_rxstart ();
    return;
  }

  /* send buffer out */
  router_output ();

  uip_len = 0;
}
