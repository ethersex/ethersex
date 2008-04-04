/* vim:fdm=marker et ai
 * {{{
 *
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
 }}} */

#include "../config.h"
#include "../uip/uip.h"
#include "../spi.h"
#include "../net/rfm12_raw_net.h"
#include "rfm12.h"

void
rfm12_process (void)
{
#ifdef ENC28J60_SUPPORT
  int recv_len = rfm12_rxfinish (uip_buf + RFM12_BRIDGE_OFFSET);

  if (recv_len == 0 || recv_len >= 254)
    return;			/* receive error or no data */

#ifdef RFM12_RAW_SUPPORT
  if (rfm12_raw_conn->rport) {
    /* rfm12 raw capturing active, forward in udp/ip encapsulated form,
       thusly don't push to the stack. */
    uip_stack_set_active (STACK_MAIN);
    memmove (uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN,
	     uip_buf + RFM12_BRIDGE_OFFSET, recv_len);
    uip_slen = recv_len;
    uip_udp_conn = rfm12_raw_conn;
    uip_process(UIP_UDP_SEND_CONN);
    fill_llh_and_transmit();
    
    rfm12_rxstart ();
    return;
  }
#endif /* RFM12_RAW_SUPPORT */

  /* uip_input expects the number of bytes including the LLH. */
  uip_len = recv_len + RFM12_BRIDGE_OFFSET;

  /* Push data into inner uIP stack. */
  uip_stack_set_active (STACK_RFM12);
  rfm12_stack_process (UIP_DATA);

#else /* not ENC28J60_SUPPORT */
  uip_len = rfm12_rxfinish (uip_buf);
  
  if (uip_len == 0 || uip_len >= 254)
    return;			/* receive error or no data */

#ifdef RFM12_BEACON_SUPPORT
  if (uip_buf[0] == 0x23 && uip_buf[2] == 0x42) {
    /* we've received a beacon, store its id. */
    rfm12_beacon_code = uip_buf[1];
    rfm12_rxstart ();
    return;
  }
#endif /* RFM12_BEACON_SUPPORT */

  uip_input ();
#endif /* not ENC28J60_SUPPORT */

  if (uip_len == 0)
    {
      rfm12_rxstart ();
      return;			/* The stack didn't generate any data
				   that has to be sent back. */
    }

  /* Application has generated output, send it out. */
  fill_llh_and_transmit ();
}

