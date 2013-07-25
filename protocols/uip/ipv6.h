/*
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
 */

#ifndef UIP_IPV6_H
#define UIP_IPV6_H

/* prototypes */

/* Prepend ethernet header to outbound IPv6 packet. */
int uip_neighbor_out(void);

/* Queue `Router Solicitation' packet. */
void uip_router_send_solicitation(void);

/* Assign IPv6 autoconfiguration address for own MAC address. */
void uip_ip6autoconfig(uint16_t addr0, uint16_t addr1,
		       uint16_t addr2, uint16_t addr3);

/* Parse an ICMPv6 router advertisement. */
void uip_router_parse_advertisement(void);


/**
 * The ICMPv6 Router Advertisement header.
 */
struct uip_icmp_radv_hdr {
  /* IPv6 header. */
  u8_t vtc,
    tcf;
  u16_t flow;
  u8_t len[2];
  u8_t proto, ttl;
  uip_ip6addr_t srcipaddr, destipaddr;
  
  /* ICMP (echo) header. */
  u8_t type, icode;
  u16_t icmpchksum;

  u8_t hoplimit;
  u8_t flags;
  u16_t router_lifetime;
  u16_t reachable[2];
  u16_t retrans[2];

  /* used as a pointer to the body */
  u8_t first_type;
};

struct uip_icmp_radv_prefix {
    u8_t type;
    u8_t length;
    u8_t prefix_length;
    u8_t flags;
    u16_t valid_lifetime[2];
    u16_t preferred_lifetime[2];
    u8_t reserved[4];
    u8_t prefix[16];
};

struct uip_icmp_radv_source {
    u8_t type;
    u8_t length;
    u8_t mac[6];
};


#endif /* UIP_IPV6_H */
