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

#include "uip.h"
#include "uip_arp.h"
#include "uip_neighbor.h"

#include "../rfm12/rfm12.h"

uint8_t 
fill_llh_and_transmit(void)
{
#ifdef RFM12_SUPPORT
#ifdef ENC28J60_SUPPORT
  if (uip_stack_get_active() == STACK_RFM12) {
#endif /* ENC28J60_SUPPORT */

    /* fill rfm12 llh, i.e. fill beacon */
    uip_buf[RFM12_BRIDGE_OFFSET] = rfm12_beacon_code;

    /* uip_len is set to the number of data bytes to be sent including
       the UDP/IP header, i.e. not including any byte for LLH. */
    rfm12_txstart (uip_buf + RFM12_BRIDGE_OFFSET, uip_len + 1);
    return 0;
#ifdef ENC28J60_SUPPORT
  }
#endif /* ENC28J60_SUPPORT */
#endif /* RFM12_SUPPORT */

#if defined(ENC28J60_SUPPORT) && defined(ZBUS_SUPPORT)
  if (uip_stack_get_active() == STACK_ZBUS) {
    /* uip_len is set to the number of data bytes to be sent including
       the UDP/IP header, i.e. not including any byte for LLH. */
    zbus_send_data (uip_buf + ZBUS_BRIDGE_OFFSET, uip_len);
    return 0;
  }
#endif /* ENC28J60_SUPPORT && ZBUS_SUPPORT */

  /*
   * ENC28J60 outbound processing ....
   */
#ifdef ENC28J60_SUPPORT
#ifdef OPENVPN_SUPPORT
  if (uip_stack_get_active() == STACK_MAIN)
    openvpn_process_out();
  /* uip_stack_set_active(STACK_OPENVPN); */
#endif

#if UIP_CONF_IPV6
  uint8_t rv = uip_neighbor_out();
#else
  uint8_t rv = uip_arp_out();
#endif
  
  transmit_packet();

  return rv;
#endif /* ENC28J60_SUPPORT */
}

