/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef UIP_ROUTER_H
#define UIP_ROUTER_H

#include "uip-conf.h"

#if defined(ROUTER_SUPPORT) && UIP_MULTI_STACK
void router_input(uint8_t stack);

/* Route a packet via STACK.  Returns 1 if the packet has been replaced
   by an arp request.  0 otherwise. */
uint8_t router_output_to (uint8_t stack);

uint8_t router_find_stack(uip_ipaddr_t *forwardip);

/* Find a suitable stack to transmit the packet in uip_buf and finally
   send it.
   This function is only used by applications, not by the stack inputs 
   */
void router_output(void);

#else

/* No routing support, simply pass packet to uip_input of the stack
   in question.*/
#define router_input(stack)			\
  do {						\
    uip_stack_set_active(stack);		\
    uip_input();				\
  } while(0)


#if defined(ENC28J60_SUPPORT)
#  include "network.h"
#  define router_output() enc28j60_txstart()

#elif defined(TAP_SUPPORT)
#  include "core/host/tap.h"
#  define router_output() tap_txstart()

#elif defined(RFM12_IP_SUPPORT)
#  include "hardware/radio/rfm12/rfm12.h"
#  define router_output() (rfm12_txstart (uip_len), 0)

#elif defined(ZBUS_SUPPORT)
#  include "protocols/zbus/zbus.h"
#  define router_output() (zbus_txstart (uip_len));

#elif defined(USB_NET_SUPPORT)
#  include "protocols/usb/usb_net.h"
#  define router_output() (usb_net_txstart(), 0)

#endif

#endif	/* ROUTER_SUPPORT && UIP_MULTI_STACK */

#endif	/* UIP_ROUTER_H */
