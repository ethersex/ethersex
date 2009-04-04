/*
 * Copyright (c) 2007, 2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "../config.h"
#include "../uip/uip.h"
#include "../uip/uip_router.h"
#include "../spi.h"
#include "rfm12_raw_net.h"
#include "rfm12.h"

void
rfm12_process (void)
{
  uip_len = rfm12_rxfinish ();
  if (! uip_len)
    return;

#ifdef ROUTER_SUPPORT
#ifdef RFM12_RAW_SUPPORT
  if (rfm12_raw_conn->rport) {
    /* rfm12 raw capturing active, forward in udp/ip encapsulated form,
       thusly don't push to the stack. */
    /* FIXME This way we cannot accept rfm12_raw requests from anything
       but ethernet.  This shalt be improved somewhen. */
    uip_stack_set_active (STACK_ENC);
    memmove (uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN, rfm12_data, uip_len);
    uip_slen = uip_len;
    uip_udp_conn = rfm12_raw_conn;
    uip_process(UIP_UDP_SEND_CONN);
    router_output ();

    uip_buf_unlock ();
    rfm12_rxstart ();
    return;
  }
#endif /* RFM12_RAW_SUPPORT */

  /* uip_input expects the number of bytes including the LLH. */
  uip_len = uip_len + RFM12_BRIDGE_OFFSET + RFM12_LLH_LEN;
#endif /* not ROUTER_SUPPORT */

  rfm12_rxstart ();

  router_input (STACK_RFM12);

  if (uip_len == 0)
    {
      uip_buf_unlock ();
      return;			/* The stack didn't generate any data
				   that has to be sent back. */
    }

  /* Application has generated output, send it out. */
  router_output ();
}

