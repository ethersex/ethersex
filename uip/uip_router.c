/* vim:fdm=marker et ai
 * {{{
 *
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
 }}} */

#include "uip_router.h"

#ifdef ROUTER_SUPPORT

#include "uip.h"
#include "uip_arp.h"
#include "uip_neighbor.h"

#include "../ipv6.h"
#include "../network.h"
#include "../rfm12/rfm12.h"
#include "../zbus/zbus.h"
#include "../usb/usb_net.h"

#ifdef IPCHAIR_SUPPORT
#define IPCHAIR_HEADER
#include "../ipchair/ipchair.c"
#undef IPCHAIR_HEADER
#endif

#ifdef DEBUG_ROUTER
# include "../debug.h"
# define printf  debug_printf
#else
# define printf(a...)
#endif

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

uint8_t 
router_find_stack(uip_ipaddr_t *forwardip)
{
  uint8_t i;
routing_input:
  for (i = 0; i < STACK_LEN; i++) {
    if((! forwardip) && uip_ipaddr_cmp(BUF->destipaddr, uip_stacks[i].uip_hostaddr))
      return i;
#ifdef IPV6_SUPPORT
    if(forwardip && uip_ipaddr_prefixlencmp(forwardip, uip_stacks[i].uip_hostaddr,
                                          uip_stacks[i].uip_prefix_len))
      return i;
#else /* !UIP_CONF_IPV6 */
    if(forwardip && uip_ipaddr_maskcmp(forwardip, uip_stacks[i].uip_hostaddr,
                                     uip_stacks[i].uip_netmask))
       return i;
#endif
  }
  /* we didn't find an interface for the forwadip, so try it again with the
   * default router
   */
  if (forwadip && forwardip != uip_draddr){
    forwardip = uip_draddr;
    goto routing_input;
  }

  /* Drop the packet */
  uip_len = 0;
  return 0;
}


void
router_input(uint8_t origin)
{
#ifdef IPCHAIR_HAVE_PREROUTING
  ipchair_PREROUTING_chair();
  if(!uip_len) return;
#endif
  /* uip_len is set to the number of received bytes, including the LLH.
     For RFM12, ZBus, etc.  it's the full 14-byte Ethernet LLH even also. */

  /* Check if packet is addressed to one stack's
     configured host address. */
  uint8_t dest = router_find_stack(NULL);
  if (dest < 255) {
      uip_stack_set_active(dest);
#ifdef IPCHAIR_HAVE_INPUT
      ipchair_PREROUTING_chair();
      if(!uip_len) return;
#endif
      uip_input ();
  }
#if UIP_CONF_IPV6 && defined(ENC28J60_SUPPORT)
  else if (BUF->destipaddr[0] == HTONS(0xff02))
    {
      /* Packet is addressed to one of the multicast addresses. */
      uip_stack_set_active (STACK_ENC);
      uip_input ();
    }
#endif	/* UIP_CONF_IPV6 */
  else
    {
#ifdef IP_FORWARDING_SUPPORT
      /* Packet not addressed to us, check destination address to where
	 the packet has to be routed. */
      uint8_t dest = router_find_stack(BUF->destipaddr);
      if (!uip_len) return; /* Packet was dropped by the router */

      if (origin == dest)
	goto drop;

      if (-- BUF->ttl == 0)
	{
	  /* TODO send ICMP message */
	  printf ("ttl exceeded, should send ICMP message.\n");
	  goto drop;
	}

#ifdef IPCHAIR_HAVE_FORWARD
      ipchair_FORWARD_chair();
      if(!uip_len) return;
#endif

#if !UIP_CONF_IPV6
      /* For IPv4 we must adjust the chksum */
      if(BUF->ipchksum >= HTONS(0xffff - (1 << 8)))
	BUF->ipchksum += HTONS(1 << 8) + 1;

      else
	BUF->ipchksum += HTONS(1 << 8);
#endif

      /* For router_output_to uip_len must be set to the number of
	 bytes to send, excluding the LLH (since it'll generate the needed
	 one itself).  However uip_len is currently set to the number of
	 received bytes, i.e. including the LLH. */
      uip_len -= UIP_LLH_LEN;

      /* TODO check MTU and send suitable ICMP message if needed. */
      router_output_to (dest);

#endif /* IP_FORWARDING_SUPPORT */

      goto drop;
    }

  return;

 drop:
  uip_len = 0;
  return;
}

void 
router_output(void) {
#ifdef IPCHAIR_HAVE_OUTPUT
  ipchair_OUTPUT_chair();
#endif

  uint8_t dest = router_find_stack(BUF->destipaddr);
  if(!uip_len) return;

  router_output_to(dest);
}

uint8_t
router_output_to (uint8_t dest)
{
  uint8_t retval = 0;

  uip_stack_set_active (dest);

#ifdef IPCHAIR_HAVE_POSTROUTING
  ipchair_POSTROUTING_chair();
  if(!uip_len) return 0;
#endif

  switch (dest)
    {

#ifdef ENC28J60_SUPPORT
    case STACK_ENC:
      printf ("router_output_to: ENC28J60.\n");
      enc28j60_txstart ();
      break;
#endif	/* ENC28J60_SUPPORT */


#ifdef RFM12_SUPPORT
    case STACK_RFM12:
      printf ("router_output_to: RFM12.\n");
      rfm12_txstart (uip_len);
      break;
#endif	/* RFM12_SUPPORT */


#ifdef ZBUS_SUPPORT
    case STACK_ZBUS:
      printf ("router_output_to: ZBUS.\n");
      zbus_txstart (uip_len);
      break;
#endif	/* ZBUS_SUPPORT */


#ifdef USB_NET_SUPPORT
    case STACK_USB:
      printf ("router_output_to: USB.\n");
      usb_net_txstart ();
      break;
#endif	/* USB_NET_SUPPORT */


#ifdef OPENVPN_SUPPORT
    case STACK_OPENVPN:
      printf ("router_output_to: OpenVPN.\n");
      openvpn_txstart ();
      break;
#endif  /* OPENVPN_SUPPORT */

    }

  return retval;
}


#endif	/* ROUTER_SUPPORT */
