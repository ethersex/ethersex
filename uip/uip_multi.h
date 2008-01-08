/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef UIP_MULTI_H
#define UIP_MULTI_H

struct uip_stack {
  /* mapped functions */
  void (* uip_process) (u8_t flag);
  void (* uip_send)    (const void *data, int len);
  u16_t (* upper_layer_chksum) (u8_t proto);

  u8_t *uip_flags;
  void **uip_appdata;
  void **uip_sappdata;

  uip_conn_t **uip_conn;
  uip_udp_conn_t **uip_udp_conn;

  uip_ipaddr_t *uip_hostaddr;
  uip_ipaddr_t *uip_netmask;
  uip_ipaddr_t *uip_draddr;

  u16_t *uip_slen;

  struct uip_stats *uip_stat;  
};

#define STACK_PROTOTYPES(stackname)					\
  uip_conn_t * stackname ## _connect					\
    (uip_ipaddr_t *ripaddr, u16_t port, uip_conn_callback_t callback);	\
  void stackname ## _send(const void *data, int len);			\
  void stackname ## _process(u8_t flag);				\
  u16_t stackname ## _upper_layer_chksum(u8_t proto);                   \
  extern void * stackname ## _appdata;					\
  extern void * stackname ## _sappdata;					\
  extern u16_t stackname ## _slen;					\
  extern uip_conn_t * stackname ## _conn;				\
  extern uip_udp_conn_t * stackname ## _udp_conn;			\
  extern struct uip_stats stackname ## _stat;				\
  extern u8_t stackname ## _flags;					\
  extern uip_ipaddr_t stackname ## _hostaddr;				\
  extern uip_ipaddr_t stackname ## _netmask;				\
  extern uip_ipaddr_t stackname ## _draddr;

extern struct uip_stack uip_stacks[STACK_LEN];
extern struct uip_stack *uip_stack;

#define uip_stack_get_active()   (uip_stack - uip_stacks)
#define uip_stack_set_active(i)  (uip_stack = &uip_stacks[(i)])

#ifdef STACK_NAME
  /* We're now compiling a uIP stack. */
#  define uip_process      STACK_NAME(process)
#  define uip_send         STACK_NAME(send)
#  define upper_layer_chksum  STACK_NAME(upper_layer_chksum)
#  define uip_flags        STACK_NAME(flags)
#  define uip_appdata      STACK_NAME(appdata)
#  define uip_sappdata     STACK_NAME(sappdata)
#  define uip_conn         STACK_NAME(conn)
#  define uip_udp_conn     STACK_NAME(udp_conn)
#  define uip_hostaddr     STACK_NAME(hostaddr)
#  define uip_netmask      STACK_NAME(netmask)
#  define uip_draddr       STACK_NAME(draddr)
#  define uip_slen         STACK_NAME(slen)
#  define uip_stat         STACK_NAME(stat)

#else
  /* We're compiling application code (i.e. outside of uIP stack) */
#  define uip_process      (uip_stack->uip_process)
#  define uip_send         (uip_stack->uip_send)
#  define upper_layer_chksum (uip_stack->upper_layer_chksum)
#  define uip_flags        (* (uip_stack->uip_flags))
#  define uip_appdata      (* (uip_stack->uip_appdata))
#  define uip_sappdata     (* (uip_stack->uip_sappdata))
#  define uip_conn         (* (uip_stack->uip_conn))
#  define uip_udp_conn     (* (uip_stack->uip_udp_conn))
#  define uip_hostaddr     (* (uip_stack->uip_hostaddr))
#  define uip_netmask      (* (uip_stack->uip_netmask))
#  define uip_draddr       (* (uip_stack->uip_draddr))
#  define uip_slen         (* (uip_stack->uip_slen))
#  define uip_stat         (* (uip_stack->uip_stat))
#endif



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

#  define OPENVPN_TOTAL_LLH_LEN  (OPENVPN_LLH_LEN + OPENVPN_CRYPT_LLH_LEN \
				  + OPENVPN_HMAC_LLH_LEN)

#  if STACK_PRIMARY && !defined(OPENVPN_OUTER)
#    define OPENVPN_INNER
#    define UIP_CONF_LLH_LEN     OPENVPN_TOTAL_LLH_LEN
#  endif
#endif /* not OPENVPN_SUPPORT */



/* We don't have a LLH on RFM12 however we might need to pass
   the packet to ethernet, therefore 14 is simpler. */
#if defined(RFM12_SUPPORT) && defined(ENC28J60_SUPPORT)
#  ifdef OPENVPN_SUPPORT
#    define RFM12_BRIDGE_OFFSET  OPENVPN_TOTAL_LLH_LEN
#  else
#    define RFM12_BRIDGE_OFFSET  14
#  endif
#ifdef RFM12_OUTER
#  define UIP_CONF_LLH_LEN       RFM12_BRIDGE_OFFSET
#endif
#endif /* RFM12_SUPPORT && ENC28J60_SUPPORT */

/* We don't have a LLH on ZBus however we might need to pass
   the packet to ethernet, therefore 14 is simpler. */
#if defined(ZBUS_SUPPORT) && defined(ENC28J60_SUPPORT)
#  ifdef OPENVPN_SUPPORT
#    define ZBUS_BRIDGE_OFFSET  OPENVPN_TOTAL_LLH_LEN
#  else
#    define ZBUS_BRIDGE_OFFSET  14
#  endif
#ifdef ZBUS_OUTER
#  define UIP_CONF_LLH_LEN       ZBUS_BRIDGE_OFFSET
#endif
#endif /* ZBUS_SUPPORT && ENC28J60_SUPPORT */


#endif /* not UIP_MULTI_H */
