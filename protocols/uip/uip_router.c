/*
 * Copyright (c) 2008 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2026 Erik Kunze <ethersex@erik-kunze.de>
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

#include "uip_router.h"

#ifdef ROUTER_SUPPORT

#include <string.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_neighbor.h"

#include "protocols/uip/ipv6.h"
#include "network.h"
#include "hardware/radio/rfm12/rfm12.h"
#include "protocols/zbus/zbus.h"
#include "protocols/usb/usb_net.h"

#if ARCH == ARCH_HOST
#include "core/host/tap.h"
#endif

#ifdef IPCHAIR_SUPPORT
#include "ipchair/ipchair.h"
#endif

#ifdef DEBUG_ROUTER
# include "core/debug.h"
# define ROUTER_DEBUG(a...) debug_printf("router: " a)
#else
# define ROUTER_DEBUG(a...)
#endif

#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ICMPBUF ((struct uip_icmpip_hdr *)&uip_buf[UIP_LLH_LEN])

#if UIP_CONF_IPV6
/* ICMPv6 message types (only the ones we generate ourselves). */
#define ICMP_TIME_EXCEEDED 3
#else
/* ICMP message types (only the ones we generate ourselves). */
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8
#define ICMP_TIME_EXCEEDED 11
#endif

#define ICMP_ERROR_HDR_LEN 8	/* type, code, chksum, unused */
/* RFC 792: internet header plus 64 bits of the datagram */
#define ICMP_QUOTE_LEN (UIP_IPH_LEN + 8)

uint8_t
router_find_stack(uip_ipaddr_t *forwardip)
{
  uint8_t i;
routing_input:
  for (i = 0; i < STACK_LEN; i++) {
    uip_stack_set_active(i);
    if((! forwardip) && uip_ipaddr_cmp(BUF->destipaddr, uip_hostaddr))
      return i;
#ifdef IPV6_SUPPORT
    if(forwardip && uip_ipaddr_prefixlencmp(*forwardip, uip_hostaddr,
                                            uip_prefix_len))
      return i;
#else /* !UIP_CONF_IPV6 */
    if(forwardip && uip_ipaddr_maskcmp(*forwardip, uip_hostaddr,
                                       uip_netmask))
       return i;
#endif
  }
  /* we didn't find an interface for the forwadip, so try it again with the
   * default router
   */
  if (forwardip && forwardip != &uip_draddr){
    forwardip = &uip_draddr;
    goto routing_input;
  }

  /* Drop the packet */
  return 255;
}


/*
 * Send an ICMP 'time exceeded' error message back to the sender of the
 * packet currently held in uip_buf, quoting its internet header
 * plus the first 64 bits of the payload (RFC 792 for IPv4, RFC 4443
 * section 2.4 for IPv6).  The error message goes out on the interface
 * (stack) the offending packet arrived on.
 */
static void
icmp_time_exceeded (uint8_t origin)
{
  uip_ipaddr_t src;

#if UIP_CONF_IPV6
  /* Never send an ICMPv6 error in response to an ICMPv6 error message,
     error types are < 128, see RFC 4443, section 2.4 (a). */
  if (BUF->proto == UIP_PROTO_ICMP6 && ICMPBUF->type < 128)
    return;
#else
  /* Never send an ICMP error in response to an ICMP error message,
     see RFC 1812, section 4.3.2.7. */
  if (BUF->proto == UIP_PROTO_ICMP
      && ICMPBUF->type != ICMP_ECHO && ICMPBUF->type != ICMP_ECHO_REPLY)
    return;
#endif

  ROUTER_DEBUG("sending time exceeded message.\n");

  uip_ipaddr_copy(src, BUF->srcipaddr);

  /* Move internet header plus quoted payload of the offending packet
     behind our own IP and ICMP headers. */
  memmove (&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN + ICMP_ERROR_HDR_LEN],
           &uip_buf[UIP_LLH_LEN], ICMP_QUOTE_LEN);

#if UIP_CONF_IPV6
  /* Build a fresh IPv6 header for the error message.  Unlike the
     IPv4 total length field, the IPv6 payload length excludes the
     40-octet header itself. */
  BUF->vtc = 0x60;
  BUF->tcflow = 0;
  BUF->flow = 0;
  BUF->len[0] =
    ((ICMP_ERROR_HDR_LEN + ICMP_QUOTE_LEN) >> 8) & 0xff;
  BUF->len[1] = (ICMP_ERROR_HDR_LEN + ICMP_QUOTE_LEN) & 0xff;
  BUF->ttl = UIP_TTL;
  BUF->proto = UIP_PROTO_ICMP6;
#else
  /* Build a fresh IPv4 header for the error message. */
  BUF->vhl = 0x45;
  BUF->tos = 0;
  BUF->len[0] =
    ((UIP_IPH_LEN + ICMP_ERROR_HDR_LEN + ICMP_QUOTE_LEN) >> 8) & 0xff;
  BUF->len[1] = (UIP_IPH_LEN + ICMP_ERROR_HDR_LEN + ICMP_QUOTE_LEN) & 0xff;
  BUF->ipid[0] = BUF->ipid[1] = 0;
  BUF->ipoffset[0] = BUF->ipoffset[1] = 0;
  BUF->ttl = UIP_TTL;
  BUF->proto = UIP_PROTO_ICMP;
#endif

  /* The source address has to be one of our addresses on the
     interface the packet was received from, RFC 1812, section
     4.3.2.3 (IPv4) and RFC 4443, section 2.4 (IPv6). */
  uip_stack_set_active (origin);
  uip_ipaddr_copy(BUF->srcipaddr, uip_hostaddr);
  uip_ipaddr_copy(BUF->destipaddr, src);

  /* Build the ICMP header. */
  ICMPBUF->type = ICMP_TIME_EXCEEDED;
#if UIP_CONF_IPV6
  ICMPBUF->icode = 0;               /* hop limit exceeded in transit */
  ICMPBUF->flags = 0;               /* unused */
  ICMPBUF->reserved1 = 0;           /* unused */
  ICMPBUF->reserved2 = 0;           /* unused */
  ICMPBUF->reserved3 = 0;           /* unused */

  ICMPBUF->icmpchksum = 0;
  ICMPBUF->icmpchksum = ~upper_layer_chksum (UIP_PROTO_ICMP6);
#else
  ICMPBUF->icode = 0;               /* ttl exceeded in transit */
  ICMPBUF->id = 0;                  /* unused */
  ICMPBUF->seqno = 0;               /* unused */

  ICMPBUF->icmpchksum = 0;
  ICMPBUF->icmpchksum = ~(uip_cksum (0, &ICMPBUF->type,
                                     ICMP_ERROR_HDR_LEN +
                                     ICMP_QUOTE_LEN));

  BUF->ipchksum = 0;
  BUF->ipchksum = ~(uip_cksum (0, BUF, UIP_IPH_LEN));
#endif

  /* uip_len excludes the link-layer header, router_output_to
     generates it itself. */
  uip_len = (UIP_IPH_LEN + ICMP_ERROR_HDR_LEN + ICMP_QUOTE_LEN);
  router_output_to (origin);
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
      ipchair_INPUT_chair();
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
      uint8_t dest = router_find_stack(&BUF->destipaddr);
      if (dest == 255)
        {
	  uip_len = 0;
          return; /* Packet was dropped by the router */
        }

      if (origin == dest)
	goto drop;

      /* Decrement the TTL, making us a visible router on the path
         (RFC 1812, section 5.2.4).  If it would reach zero the packet
         has to be destroyed and an ICMP error message sent back to
         the sender.  Note: test before decrementing, so that packets
         with a TTL of zero (or one) are caught as well. */
      if (BUF->ttl <= 1)
	{
	  icmp_time_exceeded (origin);
	  goto drop;
	}
      -- BUF->ttl;

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

  uint8_t dest = router_find_stack(&BUF->destipaddr);
  if (dest == 255)
    {
      uip_len = 0;
      return;
    }

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
      ROUTER_DEBUG("output to: ENC28J60.\n");
      enc28j60_txstart ();
      break;
#endif	/* ENC28J60_SUPPORT */


#ifdef RFM12_IP_SUPPORT
    case STACK_RFM12:
      ROUTER_DEBUG("output to: RFM12.\n");
      rfm12_txstart (uip_len);
      break;
#endif	/* RFM12_IP_SUPPORT */


#ifdef ZBUS_SUPPORT
    case STACK_ZBUS:
      ROUTER_DEBUG("output to: ZBUS.\n");
      zbus_txstart (uip_len);
      break;
#endif	/* ZBUS_SUPPORT */


#ifdef USB_NET_SUPPORT
    case STACK_USB:
      ROUTER_DEBUG("output to: USB.\n");
      usb_net_txstart ();
      break;
#endif	/* USB_NET_SUPPORT */


#ifdef OPENVPN_SUPPORT
    case STACK_OPENVPN:
      ROUTER_DEBUG("output to: OpenVPN.\n");
      openvpn_txstart ();
      break;
#endif  /* OPENVPN_SUPPORT */

#ifdef TAP_SUPPORT
    case STACK_TAP:
      ROUTER_DEBUG("output to: TAP.\n");
      tap_txstart ();
      break;
#endif  /* TAP_SUPPORT */

    }

  return retval;
}


#endif	/* ROUTER_SUPPORT */
