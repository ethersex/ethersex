dnl
dnl masquerade.m4
dnl
dnl   Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by 
dnl   the Free Software Foundation; either version 2 of the License, or
dnl   (at your option) any later version.
dnl  
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl  
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl  
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl

dnl This packet allows to masquerade packets received via USB to be
dnl automatically masqueraded, so you're able to use your Ethersex as
dnl an USB network interface.

divert(2)dnl
static uip_ipaddr_t ipchair_masq_peer_addr;
divert(0)dnl

CHAIR(MASQUERADE)
  if (uip_stack_get_active() != STACK_USB)
    __target(RETURN)

  dnl Keep a copy of the IP address so we're able to route the packets back.
  uip_ipaddr_copy(ipchair_masq_peer_addr, BUF->srcipaddr);

  dnl Override the source IP address with the one assigned to the Ethersex.
  REWRITE_SRCADDR(enc_stack_hostaddr)
  REWRITE_CHKSUM_IP()
POLICY(ACCEPT)


CHAIR(MAYBE_DEMASQUERADE)
  if (! uip_ipaddr_cmp(BUF->destipaddr, enc_stack_hostaddr))
    __target(RETURN)  dnl packet not addressed to us, forget it

  LEG(-p, tcp, -j, MAYBE_DEMASQUERADE_TCP)
  LEG(-p, udp, -j, MAYBE_DEMASQUERADE_UDP)

#ifdef IPV6_SUPPORT
  LEG(-p, icmp6, --icmp-type, ECHO_REPLY, -j, DEMASQUERADE)
#else
  LEG(-p, icmp, --icmp-type, ECHO_REPLY, -j, DEMASQUERADE)
#endif
POLICY(RETURN)


CHAIR(MAYBE_DEMASQUERADE_TCP)
  register uip_conn_t *uip_connr = uip_conn;
  for(uip_connr = &uip_conns[0]; uip_connr <= &uip_conns[UIP_CONNS - 1];
      ++uip_connr) {
    if(uip_connr->tcpstateflags != UIP_CLOSED &&
       BUF->destport == uip_connr->lport &&
       BUF->srcport == uip_connr->rport &&
       uip_ipaddr_cmp(BUF->srcipaddr, uip_connr->ripaddr)) {
      __target(RETURN)  dnl established connection of uIP
    }
  }
  

  uint8_t c;
  for(c = 0; c < UIP_LISTENPORTS; ++c) {
    if(BUF->destport == uip_listenports[c].port)
      __target(RETURN)  dnl listening socket available ...
  }
POLICY(DEMASQUERADE)


CHAIR(MAYBE_DEMASQUERADE_UDP)
  /* Demultiplex this UDP packet between the UDP "connections". */
  for(uip_udp_conn = &uip_udp_conns[UIP_UDP_CONNS - 1];
      uip_udp_conn >= &uip_udp_conns[0];
      --uip_udp_conn) {
    /* If the local UDP port is non-zero, the connection is considered
       to be used. If so, the local port number is checked against the
       destination port number in the received packet. If the two port
       numbers match, the remote port number is checked if the
       connection is bound to a remote port. Finally, if the
       connection is bound to a remote IP address, the source IP
       address of the packet is checked. */
    if(uip_udp_conn->lport != 0 &&
       BUF_UDP->destport == uip_udp_conn->lport &&
       (uip_udp_conn->rport == 0 ||
        BUF_UDP->srcport == uip_udp_conn->rport) &&
       uip_ipaddr_cmp(BUF->srcipaddr, uip_udp_conn->ripaddr)) {
      __target(RETURN)  dnl connection belongs to uIP
    }
  }
POLICY(DEMASQUERADE)


CHAIR(DEMASQUERADE)
  REWRITE_DESTADDR(ipchair_masq_peer_addr)
  REWRITE_CHKSUM_IP()
POLICY(ACCEPT)

