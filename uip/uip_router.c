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

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])


#define input_test(addr,stack)				\
  if(uip_ipaddr_cmp(BUF->destipaddr, addr ##_hostaddr))	\
    {							\
      uip_stack_set_active (stack);			\
      uip_input ();					\
    }

#if UIP_CONF_IPV6
#define forward_test(prefix,stack)					\
  if(uip_ipaddr_prefixlencmp(BUF->destipaddr, prefix ## _hostaddr,	\
                             prefix ## _prefix_len))			\
    dest = stack;
#else /* !UIP_CONF_IPV6 */
#define forward_test(prefix,stack)				\
  if(uip_ipaddr_maskcmp(BUF->destipaddr, prefix ## _hostaddr,	\
			prefix ## _netmask))			\
    dest = stack;
#endif

#define enc_test(func)              func(mainstack, STACK_MAIN)

#ifdef OPENVPN_SUPPORT
#define openvpn_test(func)          func(openvpn, STACK_OPENVPN)
#else
#define openvpn_test(func)          if(0);
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


#define chain(func)				\
  enc_test (func)				\
  else openvpn_test (func)			\
  else rfm12_test (func)			\
  else zbus_test (func)


void
router_input(uint8_t origin)
{
  /* Input */
  chain (input_test)		/* Check if packet is address to one stack's
				   configured host address. */
#if UIP_CONF_IPV6 
  else if (BUF->destipaddr[0] == HTONS(0xff02)
#if UIP_CONF_IPV6_LLADDR    
	   || uip_ipaddr_cmp(BUF->destipaddr, uip_lladdr)
#endif
	   )
    {
      /* Packet is addressed to either the link-local address of the ethernet
	 stack or to one of the multicast addresses. */ 
      uip_stack_set_active (STACK_MAIN);
      uip_input ();
    }
#endif	/* UIP_CONF_IPV6 */
  else
    {
      /* Packet not addressed to us, check destination address to where
	 the packet has to be routed. */
      uint8_t dest;

      chain (forward_test)
      else 
	{
	  /* Unknown network, use default route, i.e. ethernet */
	  dest = STACK_MAIN;	/* XXX or OpenVPN? */
	}

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

      /* TODO check MTU and send suitable ICMP message if needed. */
      uip_stack_set_active (dest);
      fill_llh_and_transmit ();
      goto drop;
    }

  return;

 drop:
  uip_len = 0;
  return;
}




#endif	/* ROUTER_SUPPORT */
