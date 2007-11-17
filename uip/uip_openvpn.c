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


/* We're now compiling the outer side of the uIP stack */
#include "uip_openvpn.h"
#include "uip.c"

void 
openvpn_handle_udp (void)
{
  if (uip_udp_conn->lport != HTONS(OPENVPN_PORT))
    return;

  /* Push data into inner uIP stack. */
  real_uip_process (UIP_DATA);

  if (! uip_len)
    return;			/* Inner stack hasn't created a
				   packet. */

  openvpn_slen = uip_len;	/* Make sure openvpn_process sends
				   the data. */
}


/* Prepare data from inner uIP stack to be sent out to the remote host,
   this is fill the IP and UDP headers of the outer stack part.  */
void
openvpn_process_out (void)
{
  /* uip_len is shared between both stacks.  uip_process (from the
     inner stack) has set it to the amount of data to be tunnelled
     (including TCP, etc. headers).  */
  if (! uip_len)
    return;			/* no data to be sent out. */

  openvpn_slen = uip_len;

  /* We assume that openvpn_udp_conns[0] always is the OpenVPN
     connection.  */
  openvpn_udp_conn = &openvpn_udp_conns[0];
  openvpn_process (UIP_UDP_SEND_CONN);
}


void 
openvpn_init (void)
{
  openvpn_uip_init ();

  /* Initialize OpenVPN stack IP config, if necessary. */
# if !UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)
  CONF_OPENVPN_IP4;
  uip_sethostaddr(ip);

  CONF_OPENVPN_IP4_NETMASK;
  uip_setnetmask(ip);

  CONF_OPENVPN_IP4_GATEWAY;
  uip_setdraddr(ip);
# endif /* not UIP_CONF_IPV6 and not BOOTP */

# if UIP_CONF_IPV6
  uip_ip6autoconfig(0xFE80, 0x0000, 0x0000, 0x0000);
# if UIP_CONF_IPV6_LLADDR
  uip_ipaddr_copy(uip_lladdr, uip_hostaddr);
# endif
# endif

  /* Create OpenVPN UDP listener. */
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  struct uip_udp_conn *openvpn_conn = uip_udp_new(&ip, 0, openvpn_handle_udp);

  if(! openvpn_conn) 
    return;					/* dammit. */

  uip_udp_bind(openvpn_conn, HTONS(OPENVPN_PORT));
}
