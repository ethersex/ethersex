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

#include <string.h>

#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/uip_neighbor.h"
#include "ipv6.h"

#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ETHBUF ((struct uip_eth_hdr *)&uip_buf[0])
#define ICMPBUF ((struct uip_icmpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define RADVBUF ((struct uip_icmp_radv_hdr *)&uip_buf[UIP_LLH_LEN])

/* Our MAC address, defined in uip/uip.c */
extern struct uip_eth_addr uip_ethaddr;

/* Calculate ICMP6 Checksum, exported from uip/uip.c */
extern u16_t uip_icmp6chksum(void);

#if UIP_CONF_IPV6

static void 
uip_neighbor_send_solicitation(uip_ipaddr_t ipaddr)
{
  uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];
  uip_len = UIP_LLH_LEN + UIP_IPH_LEN + 32;
  memset(uip_buf, 0, uip_len);

  ETHBUF->dest.addr[0] = 0x33;
  ETHBUF->dest.addr[1] = 0x33;
  ETHBUF->dest.addr[2] = 0xff;
  ETHBUF->dest.addr[3] = HTONS(ipaddr[6]) & 0xFF;
  ETHBUF->dest.addr[4] = (HTONS(ipaddr[7]) >> 8) & 0xFF;
  ETHBUF->dest.addr[5] = HTONS(ipaddr[7]) & 0xFF;

  memcpy(ETHBUF->src.addr, uip_ethaddr.addr, 6);
  ETHBUF->type = HTONS(UIP_ETHTYPE_IP6);

  ICMPBUF->vtc = 0x60;
  // ICMPBUF->tcf = 0x00;
  // ICMPBUF->flow = 0;
  // ICMPBUF->len[0] = 0;
  ICMPBUF->len[1] = 32;
  ICMPBUF->proto = UIP_PROTO_ICMP6;
  ICMPBUF->ttl = 255;
    
  uip_ipaddr_copy(ICMPBUF->srcipaddr, uip_hostaddr);
  ICMPBUF->destipaddr[0] = HTONS(0xFF02);
  // ICMPBUF->destipaddr[1] = 0;
  // ICMPBUF->destipaddr[2] = 0;
  // ICMPBUF->destipaddr[3] = 0;
  // ICMPBUF->destipaddr[4] = 0;
  ICMPBUF->destipaddr[5] = HTONS(0x0001);
  ICMPBUF->destipaddr[6] = ipaddr[6] | HTONS(0xFF00);
  ICMPBUF->destipaddr[7] = ipaddr[7];
    
  ICMPBUF->type = 135;  /* neighbour solicitation */
  // ICMPBUF->icode = 0;
  // ICMPBUF->icmpchksum = 0;
  // ICMPBUF->flags = 0;
  // ICMPBUF->reserved1 = 0;
  // ICMPBUF->reserved2 = 0;
  // ICMPBUF->reserved3 = 0;
  memcpy(ICMPBUF->icmp6data, ipaddr, 16);

  ICMPBUF->options[0] = 1; /* type: 1, aka source link layer address */
  ICMPBUF->options[1] = 1; /* length: 8 bytes */
  memcpy(ICMPBUF->options + 2, uip_ethaddr.addr, 6);

  /* Calculate checksum */
  ICMPBUF->icmpchksum = ~uip_icmp6chksum();
}


void 
uip_router_send_solicitation(void)
{
  uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN];
  uip_len = UIP_LLH_LEN + UIP_IPH_LEN + 16;
  memset(uip_buf, 0, uip_len);

  ETHBUF->dest.addr[0] = 0x33;
  ETHBUF->dest.addr[1] = 0x33;
  // ETHBUF->dest.addr[2] = 0x00;
  // ETHBUF->dest.addr[3] = 0x00;
  // ETHBUF->dest.addr[4] = 0x00;
  ETHBUF->dest.addr[5] = 0x02;

  memcpy(ETHBUF->src.addr, uip_ethaddr.addr, 6);
  ETHBUF->type = HTONS(UIP_ETHTYPE_IP6);

  ICMPBUF->vtc = 0x60;
  // ICMPBUF->tcf = 0x00;
  // ICMPBUF->flow = 0;
  // ICMPBUF->len[0] = 0;
  ICMPBUF->len[1] = 16;
  ICMPBUF->proto = UIP_PROTO_ICMP6;
  ICMPBUF->ttl = 255;
    
  uip_ipaddr_copy(ICMPBUF->srcipaddr, uip_hostaddr);
  ICMPBUF->destipaddr[0] = HTONS(0xFF02);
  // ICMPBUF->destipaddr[1] = 0;
  // ICMPBUF->destipaddr[2] = 0;
  // ICMPBUF->destipaddr[3] = 0;
  // ICMPBUF->destipaddr[4] = 0;
  // ICMPBUF->destipaddr[5] = 0;
  // ICMPBUF->destipaddr[6] = 0;
  ICMPBUF->destipaddr[7] = HTONS(0x0002);
    
  ICMPBUF->type = 133;  /* router solicitation */
  // ICMPBUF->icode = 0;
  // ICMPBUF->icmpchksum = 0;
  // ICMPBUF->flags = 0;
  // ICMPBUF->reserved1 = 0;
  // ICMPBUF->reserved2 = 0;
  // ICMPBUF->reserved3 = 0;

  ICMPBUF->icmp6data[0] = 1; /* type: 1, aka source link layer address */
  ICMPBUF->icmp6data[1] = 1; /* length: 8 bytes */
  memcpy(ICMPBUF->icmp6data + 2, uip_ethaddr.addr, 6);

  /* Calculate checksum */
  ICMPBUF->icmpchksum = ~uip_icmp6chksum();
}


void
uip_ip6autoconfig(uint16_t addr0, uint16_t addr1,
		  uint16_t addr2, uint16_t addr3)
{
  uip_ipaddr_t ipaddr;

  uint8_t i = 
  i = (i & 0xFD) | ((i & 2) ? 0 : 2);
  
  uint16_t addr4 = ((uip_ethaddr.addr[0] ^ 2) << 8) | uip_ethaddr.addr[1];
  uint16_t addr5 = (uip_ethaddr.addr[2] << 8) | 0xFF;
  uint16_t addr6 = 0xFE00 | uip_ethaddr.addr[3];
  uint16_t addr7 = (uip_ethaddr.addr[4] << 8) | uip_ethaddr.addr[5];

  uip_ip6addr(ipaddr, addr0, addr1, addr2, addr3, addr4, addr5, addr6, addr7);
  uip_sethostaddr(ipaddr);
}


/* Parse an ICMPv6 router advertisement. */
void
uip_router_parse_advertisement(void)
{

  /* check that first option is `prefix information'. */
  if(RADVBUF->prefix.type != 3)
    goto error_out;
  if(RADVBUF->prefix.length != 4)
    goto error_out;
  if(RADVBUF->prefix.prefix_length != 64)
    goto error_out;

  /* check that second option is `source link layer address'. */
  if(RADVBUF->source.type != 1)
    goto error_out;
  if(RADVBUF->source.length != 1)
    goto error_out;

  /* packet looks sane, update configuration */
  uip_ip6autoconfig
    ((RADVBUF->prefix.prefix[0] << 8) | RADVBUF->prefix.prefix[1],
     (RADVBUF->prefix.prefix[2] << 8) | RADVBUF->prefix.prefix[3],
     (RADVBUF->prefix.prefix[4] << 8) | RADVBUF->prefix.prefix[5],
     (RADVBUF->prefix.prefix[6] << 8) | RADVBUF->prefix.prefix[7]);

  /* cache neighbor entry of the advertising router. */
  uip_neighbor_add(ICMPBUF->srcipaddr,
		   (struct uip_neighbor_addr *) RADVBUF->source.mac);

  /* use the router's ip address as new default gateway. */
  uip_ipaddr_copy(uip_draddr, RADVBUF->prefix.prefix);
  return;

 error_out:
  return;
}


int
uip_neighbor_out(void) 
{
  uip_ipaddr_t ipaddr;

  /* Check if the destination address is on the local network. 
   * FIXME, for the moment we assume a 64-bit "netmask" */
  if(memcmp(IPBUF->destipaddr, uip_hostaddr, 8)
#    if UIP_CONF_IPV6_LLADDR
     && memcmp(IPBUF->destipaddr, uip_lladdr, 8)
#    endif
    )
    /* Remote address is not on the local network, use router */
    uip_ipaddr_copy(ipaddr, uip_draddr);

  else
    /* Remote address is on the local network, send directly. */
    uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);

  struct uip_neighbor_addr *remote_mac = uip_neighbor_lookup(ipaddr);

  if(! remote_mac) {
    /* We don't know the remote MAC so far, therefore send neighbor
     * solicitation packet. */
    uip_neighbor_send_solicitation(ipaddr);
    return 1;
  }

  /* Initialize ethernet header. */
  memcpy(ETHBUF->dest.addr, remote_mac->addr.addr, 6);
  memcpy(ETHBUF->src.addr, uip_ethaddr.addr, 6);
  ETHBUF->type = HTONS(UIP_ETHTYPE_IP6);

  uip_len += sizeof(struct uip_eth_hdr);
  return 0;
}

#endif /* UIP_CONF_IPV6 */
