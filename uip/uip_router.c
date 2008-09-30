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

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])


#define input_test(addr,stack)				\
  if(uip_ipaddr_cmp(BUF->destipaddr, addr ##_hostaddr))	\
    {							\
      uip_stack_set_active (stack);			\
      uip_input ();					\
    }

#if UIP_CONF_IPV6
#define forward_test(prefix,stack)					\
  if(uip_ipaddr_prefixlencmp(forwardip, prefix ## _hostaddr,	\
                             prefix ## _prefix_len))			\
    dest = stack;
#else /* !UIP_CONF_IPV6 */
#define forward_test(prefix,stack)				\
  if(uip_ipaddr_maskcmp(forwardip, prefix ## _hostaddr,	\
			prefix ## _netmask))			\
    dest = stack;
#endif

#ifdef ENC28J60_SUPPORT
#define enc_test(func)              func(enc_stack, STACK_ENC)
#else
#define enc_test(func)              if(0);
#endif

#ifdef RFM12_SUPPORT
#define rfm12_test(func)            func(rfm12_stack, STACK_RFM12)
#else
#define rfm12_test(func)            if(0);
#endif

#ifdef ZBUS_SUPPORT
#define zbus_test(func)             func(zbus_stack, STACK_ZBUS)
#else
#define zbus_test(func)             if(0);
#endif

#ifdef USB_NET_SUPPORT
#define usb_net_test(func)          func(usb_stack, STACK_USB)
#else
#define usb_net_test(func)          if(0);
#endif

#define chain(func)				\
  rfm12_test (func)				\
  else usb_net_test (func)			\
  else zbus_test (func)				\
  else enc_test (func)


void
router_input(uint8_t origin)
{
  /* uip_len is set to the number of received bytes, including the LLH.
     For RFM12, ZBus, etc.  it's the full 14-byte Ethernet LLH even also. */

  /* Input */
  chain (input_test)		/* Check if packet is addressed to one stack's
				   configured host address. */
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
      uint8_t dest = router_find_destination ();

      if (origin == dest)
	goto drop;

      if (-- BUF->ttl == 0)
	{
	  /* TODO send ICMP message */
	  goto drop;
	}

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



uint8_t
router_find_destination (void)
{
  uint8_t dest = 0;
  /* This variable is used in chain(forward_test) */
  uip_ipaddr_t *forwardip = &BUF->destipaddr;

  while (1) {
    chain (forward_test)
    else
    {
      if (dest == 0xff) return 0; /* Reroute with default router failed */

      /* Can't find destination for this forwardip,
       * we try the default gateway */
      forwardip = &uip_draddr;
      dest = 0xff; /* Here prevent our self from running into an endless loop */
    }
    if (dest != 0xff) break;
  }

  return dest;
}


uint8_t
router_output_to (uint8_t dest)
{
  uint8_t retval = 0;
  uip_stack_set_active (dest);

  switch (dest)
    {

#ifdef ENC28J60_SUPPORT
    case STACK_ENC:
      enc28j60_txstart ();
      break;
#endif	/* ENC28J60_SUPPORT */


#ifdef RFM12_SUPPORT
    case STACK_RFM12:
      rfm12_txstart (uip_len);
      break;
#endif	/* RFM12_SUPPORT */


#ifdef ZBUS_SUPPORT
    case STACK_ZBUS:
      zbus_send_data (uip_buf + ZBUS_BRIDGE_OFFSET, uip_len);
      break;
#endif	/* ZBUS_SUPPORT */


#ifdef USB_NET_SUPPORT
    case STACK_USB:
      usb_net_txstart ();
      break;
#endif	/* USB_NET_SUPPORT */

    }

  return retval;
}


#endif	/* ROUTER_SUPPORT */
