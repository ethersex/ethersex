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

#ifndef UIP_OPENVPN_H
#define UIP_OPENVPN_H

#ifdef __UIP_CONF_H__
#error "uip_openvpn.h must be included first"
#endif

#include "../config.h"
#ifdef OPENVPN_SUPPORT

/* We're now compiling the outer side of the uIP stack */
#define OPENVPN_OUTER

#undef UIP_UDP_APPCALL
#define UIP_UDP_APPCALL     openvpn_handle_udp

#define uip_init            openvpn_uip_init
#define uip_process         openvpn_process
#define uip_send            openvpn_send
#define uip_flags           openvpn_flags
#define uip_appdata         openvpn_appdata
#define uip_sappdata        openvpn_sappdata
#define uip_conn            openvpn_conn
#define uip_draddr          openvpn_draddr
#define uip_hostaddr        openvpn_hostaddr
#define uip_netmask         openvpn_netmask
#define uip_slen            openvpn_slen
#define uip_stat            openvpn_stat
#define uip_udp_conn        openvpn_udp_conn
#define uip_udp_conns       openvpn_udp_conns
#define uip_udp_new         openvpn_udp_new

void openvpn_handle_udp(void);

#include "uip-conf.h"

/* Now override the current uIP configuration to fit the OpenVPN stack needs. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0

#undef UIP_CONF_UDP
#define UIP_CONF_UDP 1

#undef UIP_CONF_UDP_CONNS
#ifdef BOOTP_SUPPORT
#  define UIP_CONF_UDP_CONNS 2
#  undef UIP_CONF_BROADCAST
#  define UIP_CONF_BROADCAST 1
#else /* !BOOTP_SUPPORT */
#  define UIP_CONF_UDP_CONNS 1
#endif


/* The port number to use for OpenVPN. */
#define OPENVPN_PORT 1194


#endif /* OPENVPN_SUPPORT */
#endif /* UIP_OPENVPN_H */
