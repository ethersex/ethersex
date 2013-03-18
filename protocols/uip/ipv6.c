/*
 *
 * Copyright (c) 2007,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <string.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_neighbor.h"
#include "protocols/uip/uip_arp.h"
#include "services/tftp/tftp.h"
#include "services/dyndns/dyndns.h"
#include "protocols/uip/ipv6.h"
#include "config.h"
#include "core/global.h"
#include "core/debug.h"

#ifdef DEBUG_NET_IP6
# include "core/debug.h"
# define IP6DEBUG(a...)  debug_printf("ip6: " a)
#else
# define IP6DEBUG(a...)
#endif

/* force ethernet LLH, we'll never ever send solicitations over
   tunnelled lines */
#undef UIP_LLH_LEN
#define UIP_LLH_LEN (14 + VLAN_LLH_EXTRA)

#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define ETHBUF ((struct uip_eth_hdr *)&uip_buf[0])
#define ICMPBUF ((struct uip_icmpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define RADVBUF ((struct uip_icmp_radv_hdr *)&uip_buf[UIP_LLH_LEN])

/* Our MAC address, defined in uip/uip.c */
extern struct uip_eth_addr uip_ethaddr;

/* Calculate ICMP6 Checksum, exported from uip/uip.c */
#define uip_icmp6chksum() (upper_layer_chksum(UIP_PROTO_ICMP6))

#if UIP_CONF_IPV6 && defined(ETHERNET_SUPPORT)


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


#ifndef IPV6_STATIC_SUPPORT
void
uip_router_send_solicitation(void)
{
  IP6DEBUG ("emitting router solicitation.\n");

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
#endif /* not IPV6_STATIC_SUPPORT */


void
uip_ip6autoconfig(uint16_t addr0, uint16_t addr1,
		  uint16_t addr2, uint16_t addr3)
{
  uip_ipaddr_t ipaddr;

  uint16_t addr4 = ((uip_ethaddr.addr[0] ^ 2) << 8) | uip_ethaddr.addr[1];
  uint16_t addr5 = (uip_ethaddr.addr[2] << 8) | 0xFF;
  uint16_t addr6 = 0xFE00 | uip_ethaddr.addr[3];
  uint16_t addr7 = (uip_ethaddr.addr[4] << 8) | uip_ethaddr.addr[5];

  uip_ip6addr(ipaddr, addr0, addr1, addr2, addr3, addr4, addr5, addr6, addr7);

# ifdef DYNDNS_SUPPORT
  if (addr0 != 0xFE80) {
    /* Get old host address */
    uip_ipaddr_t old_ipaddr;
    uip_gethostaddr(&old_ipaddr);

    if(! uip_ipaddr_cmp(&ipaddr, &old_ipaddr)) {
      /* Update the dyndns name only if address has changed */
      dyndns_update();
    }
  }
# endif

  uip_sethostaddr(&ipaddr);

# ifdef TFTPOMATIC_SUPPORT
  const char filename[] PROGMEM = CONF_TFTP_IMAGE;
  uip_ipaddr_t ip;
  set_CONF_TFTP_IP(&ip);

  if (addr0 != 0xFE80) {
    tftp_fire_tftpomatic(&ip, filename, 1);
    bootload_delay = CONF_BOOTLOAD_DELAY;
  }
# endif /* TFTPOMATIC_SUPPORT */
}

#ifndef IPV6_STATIC_SUPPORT
/* Parse an ICMPv6 router advertisement. */
void
uip_router_parse_advertisement(void)
{
  struct uip_icmp_radv_prefix *prefix;
  struct uip_icmp_radv_source *source;

  IP6DEBUG ("parsing RA, first_type is %d.\n", RADVBUF->first_type);

  uint8_t *option = &(RADVBUF->first_type);

  /* For IPv6, uip_len gets set to the size of only the IP payload
   * (length field + 40 bytes for the header), so we add the link
   * level header length */
  uint8_t *packet_end = uip_buf + uip_len + UIP_LLH_LEN;

  /* The first byte of an ICMPv6 option is its type, the second one is
   * its length. */
  while ((option + (option[1] * 8)) <= packet_end) {
    switch (option[0]) {
      case 1:
        /* Source link-layer address */
        source = (struct uip_icmp_radv_source*)option;
        if (source->length != 1)
          goto error_out;

        /* cache neighbor entry of the advertising router. */
        uip_neighbor_add(ICMPBUF->srcipaddr,
                         (struct uip_neighbor_addr *) source->mac);
        break;
      case 3:
	/* Prefix information */
        prefix = (struct uip_icmp_radv_prefix*)option;
	if (prefix->length != 4 || prefix->prefix_length != 64)
          goto error_out;

        /* packet looks sane, update configuration */
        uip_ip6autoconfig
        ((prefix->prefix[0] << 8) | prefix->prefix[1],
         (prefix->prefix[2] << 8) | prefix->prefix[3],
         (prefix->prefix[4] << 8) | prefix->prefix[5],
         (prefix->prefix[6] << 8) | prefix->prefix[7]);

        /* test the prefix for being a netaddress ( last 8 byte are 0 )
         * if it is a prefix use the srcipaddr
         */

        uint8_t i, isnt_prefix = 0;
        for (i = 8; i < 16; i++)
          if (prefix->prefix[i] != 0)
            isnt_prefix = 1;

        if (isnt_prefix)
          /* use the router's ip address as new default gateway. */
          uip_ipaddr_copy(uip_draddr, prefix->prefix);
        else
          uip_ipaddr_copy(uip_draddr, ICMPBUF->srcipaddr);

        break;
      default:
          IP6DEBUG("Ignoring option type 0x%02x of RA\n", option[0]);
        break;
    }

    if ((option + (option[1] * 8)) == packet_end)
        break;
    option += (option[1] * 8);
  }

  return;

error_out:
  IP6DEBUG ("unabled to parse RA.\n");
  return;
}
#endif /* not IPV6_STATIC_SUPPORT */


int
uip_neighbor_out(void)
{
  uip_ipaddr_t ipaddr;
  struct uip_neighbor_addr *remote_mac;
#ifdef MDNS_SD_SUPPORT
  const uip_ipaddr_t mdns_address =
  {0x02ff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xfb00};
#endif /* MDNS_SD_SUPPORT */

  /* Check if the destination address is on the local network.
   * FIXME, for the moment we assume a 64-bit "netmask" */
  if(memcmp(IPBUF->destipaddr, uip_hostaddr, 8)) {
    if(uip_ipaddr_cmp(uip_draddr, all_zeroes_addr)) {
      /* Router-address not yet configured, kill packet, cannot transmit. */
      uip_len = 0;
      return 1;
    }

    /* Remote address is not on the local network, use router */
    uip_ipaddr_copy(ipaddr, uip_draddr);
  }
  else
    /* Remote address is on the local network, send directly. */
    uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);

#ifdef MDNS_SD_SUPPORT
  if (uip_ipaddr_cmp(IPBUF->destipaddr, mdns_address))  {
    /* The MDNS remote address will always be on the same network, so we don't
     * have to use the router */
    uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);
    /* We send the answer to the mac of the asking machine */
    memcpy(ETHBUF->dest.addr, ETHBUF->src.addr, 6);
    goto after_neighbour_resolv;
  } else
#endif /* MDNS_SD_SUPPORT */
    remote_mac = uip_neighbor_lookup(ipaddr);

  if(! remote_mac) {
    /* We don't know the remote MAC so far, therefore send neighbor
     * solicitation packet. */
    uip_neighbor_send_solicitation(ipaddr);
    return 1;
  }

  /* Initialize ethernet header. */
  memcpy(ETHBUF->dest.addr, remote_mac->addr.addr, 6);

#ifdef MDNS_SD_SUPPORT
after_neighbour_resolv:
#endif
  memcpy(ETHBUF->src.addr, uip_ethaddr.addr, 6);
  ETHBUF->type = HTONS(UIP_ETHTYPE_IP6);

  uip_len += sizeof(struct uip_eth_hdr);
  return 0;
}

#endif /* UIP_CONF_IPV6 and ETHERNET_SUPPORT */
