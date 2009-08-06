/*
 * Copyright (c) 2007, 2008, 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef UIP_MULTI_H
#define UIP_MULTI_H

struct uip_stack {
  uip_ipaddr_t *uip_hostaddr;
#ifdef IPV6_SUPPORT
  u8_t *uip_prefix_len;
#else
  uip_ipaddr_t *uip_netmask;
#endif

#ifdef IPSTATS_SUPPORT
  struct uip_stats *uip_stat;  
#endif
};

#define STACK_HACKARY(foo,stackname)					\
  foo struct uip_stats stackname ## _stat;				\
  foo uip_ipaddr_t stackname ## _hostaddr;				\
  foo uip_ipaddr_t stackname ## _netmask;				\
  foo u8_t stackname ## _prefix_len;				        

#define STACK_PROTOTYPES(stackname)  STACK_HACKARY(extern,stackname)
#define STACK_DEFINITIONS(stackname) STACK_HACKARY(,stackname)

extern struct uip_stack uip_stacks[STACK_LEN];
extern struct uip_stack *uip_stack;

#define uip_stack_get_active()   (uip_stack - uip_stacks)
#define uip_stack_set_active(i)  (uip_stack = &uip_stacks[(i)])

/* We're compiling application code (i.e. outside of uIP stack) */
#define uip_hostaddr     (* (uip_stack->uip_hostaddr))
#define uip_netmask      (* (uip_stack->uip_netmask))
#define uip_prefix_len   (* (uip_stack->uip_prefix_len))
#define uip_stat         (* (uip_stack->uip_stat))


#ifdef OPENVPN_SUPPORT
     /* The header of the link layer (of the inner stack) consists of:
      *
      *                                       IPv4          IPv6
      *
      *     actual link layer (ethernet)        14            14
      *     IP header of OpenVPN stack          20            40
      *     UDP header of OpenVPN stack          8             8
      *   ----------------------------------------------------------
      *     total                               42            62
      */
#  if UIP_CONF_IPV6
#    define OPENVPN_LLH_LEN (__LLH_LEN + 40 + 8)
#  else
#    define OPENVPN_LLH_LEN (__LLH_LEN + 20 + 8)
#  endif

#  ifdef MD5_SUPPORT
#    define OPENVPN_HMAC_LLH_LEN   16
#  else
#    define OPENVPN_HMAC_LLH_LEN   0
#  endif

#  ifdef CAST5_SUPPORT
#    define OPENVPN_CRYPT_LLH_LEN  16 /* 8 bytes IV + 8 bytes packet id */
#  else
#    define OPENVPN_CRYPT_LLH_LEN  0
#  endif

#  define OPENVPN_HMAC_CRYPT_LEN  (OPENVPN_HMAC_LLH_LEN + OPENVPN_CRYPT_LLH_LEN)
#  define OPENVPN_TOTAL_LLH_LEN  (OPENVPN_LLH_LEN + OPENVPN_HMAC_CRYPT_LEN)


#endif /* not OPENVPN_SUPPORT */

#define BASE_LLH_LEN  __LLH_LEN
#define UIP_CONF_LLH_LEN  __LLH_LEN

/* We have a one byte LLH on RFM12 however we might need to pass
   the packet to ethernet, therefore 14 is simpler. */
#define RFM12_BRIDGE_OFFSET  (BASE_LLH_LEN - RFM12_LLH_LEN)
#define ZBUS_BRIDGE_OFFSET   (BASE_LLH_LEN)
#define USB_BRIDGE_OFFSET    (BASE_LLH_LEN)


#endif /* not UIP_MULTI_H */
