/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uiparp uIP Address Resolution Protocol
 * @{
 *
 * The Address Resolution Protocol ARP is used for mapping between IP
 * addresses and link level addresses such as the Ethernet MAC
 * addresses. ARP uses broadcast queries to ask for the link level
 * address of a known IP address and the host which is configured with
 * the IP address for which the query was meant, will respond with its
 * link level address.
 *
 * \note This ARP implementation only supports Ethernet.
 */
 
/**
 * \file
 * Implementation of the ARP Address Resolution Protocol.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */

/*
 * Copyright (c) 2001-2003, Adam Dunkels.
 * Copyright (c) 2008, Stefan Siegl <stesie@brokenpipe.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip_arp.c,v 1.8 2006/06/02 23:36:21 adam Exp $
 *
 */

/* we want to live on the outer stack, if there are two ... */
#include "uip_openvpn.h"

#include "network.h"
#include "uip_arp.h"
#include "config.h"

#include <string.h>

#define flip(t,a,b)  do { t __j = a; a = b; b = __j; } while(0)

struct arp_hdr {
  struct uip_eth_hdr ethhdr;
  u16_t hwtype;
  u16_t protocol;
  u8_t hwlen;
  u8_t protolen;
  u16_t opcode;
  struct uip_eth_addr shwaddr;
  u16_t sipaddr[2];
  struct uip_eth_addr dhwaddr;
  u16_t dipaddr[2];
};

struct ethip_hdr {
  struct uip_eth_hdr ethhdr;
  /* IP header. */
  u8_t vhl,
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  u16_t ipchksum;
  u16_t srcipaddr[2],
    destipaddr[2];
};

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

struct arp_entry {
  u16_t ipaddr[2];
  struct uip_eth_addr ethaddr;
  u8_t time;
};

static const struct uip_eth_addr broadcast_ethaddr =
  {{0xff,0xff,0xff,0xff,0xff,0xff}};
static const u16_t broadcast_ipaddr[2] = {0xffff,0xffff};

static struct arp_entry arp_table[UIP_ARPTAB_SIZE];

static u8_t arptime;

#define BUF   ((struct arp_hdr *)&uip_buf[0])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the ARP module.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_init(void)
{
  for(u8_t i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    memset(arp_table[i].ipaddr, 0, 4);
  }
}
/*-----------------------------------------------------------------------------------*/
/**
 * Periodic ARP processing function.
 *
 * This function performs periodic timer processing in the ARP module
 * and should be called at regular intervals. The recommended interval
 * is 10 seconds between the calls.
 *
 */
/*-----------------------------------------------------------------------------------*/
#if !defined(BOOTLOADER_SUPPORT) && !defined(TEENSY_SUPPORT)
void
uip_arp_timer(void)
{
  struct arp_entry *tabptr;
  
  ++arptime;
  for(u8_t i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if((tabptr->ipaddr[0] | tabptr->ipaddr[1]) != 0 &&
       arptime - tabptr->time >= UIP_ARP_MAXAGE) {
      memset(tabptr->ipaddr, 0, 4);
    }
  }

}
#endif /* !BOOTLOADER_SUPPORT */
/*-----------------------------------------------------------------------------------*/
static void
uip_arp_update(u16_t *ip, struct uip_eth_addr *ethaddr)
{
  register struct arp_entry *tabptr = NULL;
  u8_t i;

  /* Walk through the ARP mapping table and try to find an entry to
     update. If none is found, the IP -> MAC address mapping is
     inserted in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {

    tabptr = &arp_table[i];
    /* Only check those entries that are actually in use. */
    if(tabptr->ipaddr[0] != 0 &&
       tabptr->ipaddr[1] != 0) {

      /* Check if the source IP address of the incoming packet matches
         the IP address in this ARP table entry. */
      if(ip[0] == tabptr->ipaddr[0] &&
	 ip[1] == tabptr->ipaddr[1]) {
	 
	/* An old entry found, update this and return. */
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;

	return;
      }
    }
  }

  /* If we get here, no existing ARP table entry was found, so we
     create one. */

  /* First, we try to find an unused entry in the ARP table. */
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if(tabptr->ipaddr[0] == 0 &&
       tabptr->ipaddr[1] == 0) {
      break;
    }
  }

  /* If no unused entry is found, we try to find the oldest entry and
     throw it away. */
  if(i == UIP_ARPTAB_SIZE) {
    u8_t tmpage = 0;
    u8_t c = 0;
    for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
      tabptr = &arp_table[i];
      if(arptime - tabptr->time > tmpage) {
	tmpage = arptime - tabptr->time;
	c = i;
      }
    }
    i = c;
    tabptr = &arp_table[i];
  }

  /* Now, i is the ARP table entry which we will fill with the new
     information. */
  memcpy(tabptr->ipaddr, ip, 4);
  memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
  tabptr->time = arptime;
}
/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming IP packets
 *
 * This function should be called by the device driver when an IP
 * packet has been received. The function will check if the address is
 * in the ARP cache, and if so the ARP cache entry will be
 * refreshed. If no ARP cache entry was found, a new one is created.
 *
 * This function expects an IP packet with a prepended Ethernet header
 * in the uip_buf[] buffer, and the length of the packet in the global
 * variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/

#ifndef BOOTLOADER_SUPPORT
void
uip_arp_ipin(void)
{
	
  /* Only insert/update an entry if the source IP address of the
     incoming IP packet comes from a host on the local network. */
#if ARCH == ARCH_HOST
  uip_stack_set_active(STACK_TAP);
#else
  uip_stack_set_active(STACK_ENC);
#endif
  if (uip_ipaddr_maskcmp(IPBUF->srcipaddr, uip_hostaddr, uip_netmask)) 
    uip_arp_update(IPBUF->srcipaddr, &(IPBUF->ethhdr.src));
  
  return;
}
#endif

/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming ARP packets.
 *
 * This function should be called by the device driver when an ARP
 * packet has been received. The function will act differently
 * depending on the ARP packet type: if it is a reply for a request
 * that we previously sent out, the ARP cache will be filled in with
 * the values from the ARP reply. If the incoming ARP packet is an ARP
 * request for our IP address, an ARP reply packet is created and put
 * into the uip_buf[] buffer.
 *
 * When the function returns, the value of the global variable uip_len
 * indicates whether the device driver should send out a packet or
 * not. If uip_len is zero, no packet should be sent. If uip_len is
 * non-zero, it contains the length of the outbound packet that is
 * present in the uip_buf[] buffer.
 *
 * This function expects an ARP packet with a prepended Ethernet
 * header in the uip_buf[] buffer, and the length of the packet in the
 * global variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void
uip_arp_arpin(void)
{
  
  if(uip_len < sizeof(struct arp_hdr)) {
    uip_len = 0;
    return;
  }
  uip_len = 0;
  
  switch(BUF->opcode) {
  case HTONS(ARP_REQUEST):
    /* ARP request. If it asked for our address, we send out a
       reply. */
    if(uip_ipaddr_cmp(BUF->dipaddr, uip_hostaddr)
#ifdef RFM12_ARP_PROXY
       /* If RFM12 ARP-proxy is enabled, check that one's IP address
	  as well and possibly answer it. */
       || uip_ipaddr_maskcmp(BUF->dipaddr, rfm12_stack_hostaddr,
			     rfm12_stack_netmask)
#endif
#ifdef ZBUS_ARP_PROXY
       /* If ZBUS ARP-proxy is enabled, check that one's IP address
	  as well and possibly answer it. */
       || uip_ipaddr_maskcmp(BUF->dipaddr, zbus_stack_hostaddr,
			     zbus_stack_netmask)
#endif
#ifdef USB_ARP_PROXY
       /* If USB ARP-proxy is enabled, check that one's IP address
	  as well and possibly answer it. */
       || uip_ipaddr_maskcmp(BUF->dipaddr, usb_stack_hostaddr,
			     usb_stack_netmask)
#endif
       ) {
      /* First, we register the one who made the request in our ARP
	 table, since it is likely that we will do more communication
	 with this host in the future. */
      uip_arp_update(BUF->sipaddr, &BUF->shwaddr);
      
      /* The reply opcode is 2. */
      BUF->opcode = HTONS(2);

      memcpy(BUF->dhwaddr.addr, BUF->shwaddr.addr, 6);
      memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->ethhdr.dest.addr, BUF->dhwaddr.addr, 6);

      for (u8_t i = 0; i < 4; i ++)
	flip (u8_t, ((u8_t *) BUF->dipaddr)[i],
	      ((u8_t *) BUF->sipaddr)[i]);

      /* BUF->dipaddr[0] = BUF->sipaddr[0];
	 BUF->dipaddr[1] = BUF->sipaddr[1];
	 BUF->sipaddr[0] = uip_hostaddr[0];
	 BUF->sipaddr[1] = uip_hostaddr[1]; */

      BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);
      uip_len = sizeof(struct arp_hdr);
    }
    break;
  case HTONS(ARP_REPLY):
    /* ARP reply. We insert or update the ARP table if it was meant
       for us. */
    if(uip_ipaddr_cmp(BUF->dipaddr, uip_hostaddr)) {
      uip_arp_update(BUF->sipaddr, &BUF->shwaddr);
    }
    break;
  }

  return;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Prepend Ethernet header to an outbound IP packet and see if we need
 * to send out an ARP request.
 *
 * This function should be called before sending out an IP packet. The
 * function checks the destination IP address of the IP packet to see
 * what Ethernet MAC address that should be used as a destination MAC
 * address on the Ethernet.
 *
 * If the destination IP address is in the local network (determined
 * by logical ANDing of netmask and our IP address), the function
 * checks the ARP cache to see if an entry for the destination IP
 * address is found. If so, an Ethernet header is prepended and the
 * function returns. If no ARP cache entry is found for the
 * destination IP address, the packet in the uip_buf[] is replaced by
 * an ARP request packet for the IP address. The IP packet is dropped
 * and it is assumed that they higher level protocols (e.g., TCP)
 * eventually will retransmit the dropped packet.
 *
 * If the destination IP address is not on the local network, the IP
 * address of the default router is used instead.
 *
 * When the function returns, a packet is present in the uip_buf[]
 * buffer, and the length of the packet is in the global variable
 * uip_len.
 */
/*-----------------------------------------------------------------------------------*/
u8_t
uip_arp_out(void)
{
#ifdef MDNS_SD_SUPPORT
  uip_ipaddr_t mdns_address = {0x00e0, 0xfb00};
#endif
  
  /* Find the destination IP address in the ARP table and construct
     the Ethernet header. If the destination IP addres isn't on the
     local network, we use the default router's IP address instead.

     If not ARP table entry is found, we overwrite the original IP
     packet with an ARP request for the IP address. */

  /* First check if destination is a local broadcast. */
  if(((const u8_t *)IPBUF->destipaddr)[0] >= 224
     && ((const u8_t *)IPBUF->destipaddr)[0] <= 239) {
    /* packet is addressed to multicast ip range, generate
       the associated mac address for it. */
    IPBUF->ethhdr.dest.addr[0] = 0x01;
    IPBUF->ethhdr.dest.addr[1] = 0x00;
    IPBUF->ethhdr.dest.addr[2] = 0x5e;
    IPBUF->ethhdr.dest.addr[3] = ((const u8_t *)IPBUF->destipaddr)[1] & 0x7f;
    IPBUF->ethhdr.dest.addr[4] = ((const u8_t *)IPBUF->destipaddr)[2];
    IPBUF->ethhdr.dest.addr[5] = ((const u8_t *)IPBUF->destipaddr)[3];
  }
  else if((IPBUF->destipaddr[0] == (uip_hostaddr[0] | ~uip_netmask[0])
           && IPBUF->destipaddr[1] == (uip_hostaddr[1] | ~uip_netmask[1]))
          || (uip_ipaddr_cmp(IPBUF->destipaddr, broadcast_ipaddr))) {
    memcpy(IPBUF->ethhdr.dest.addr, broadcast_ethaddr.addr, 6);
#ifdef MDNS_SD_SUPPORT
  /* If the ip is the mdns mulicast ip, we answer to the mac who asked */
  } else if (uip_ipaddr_cmp(IPBUF->destipaddr, mdns_address)) {
    memcpy(IPBUF->ethhdr.dest.addr, &((struct uip_eth_hdr *) uip_buf)->dest, 6); 
#endif
  } else {
    uip_ipaddr_t ipaddr;
    /* Check if the destination address is on the local network. */
    if(!uip_ipaddr_maskcmp(IPBUF->destipaddr, uip_hostaddr, uip_netmask)) {
      /* Destination address was not on the local network, so we need to
	 use the default router's IP address instead of the destination
	 address when determining the MAC address. */
      uip_ipaddr_copy(ipaddr, uip_draddr);
    } else {
      /* Else, we use the destination IP address. */
      uip_ipaddr_copy(ipaddr, IPBUF->destipaddr);
    }

    struct arp_entry *tabptr = uip_arp_lookup (ipaddr);

    if(!tabptr) {
      /* The destination address was not in our ARP table, so we
	 overwrite the IP packet with an ARP request. */

      memset(BUF->ethhdr.dest.addr, 0xff, 6);
      memset(BUF->dhwaddr.addr, 0x00, 6);
      memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
      memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);
    
      uip_ipaddr_copy(BUF->dipaddr, ipaddr);
      uip_ipaddr_copy(BUF->sipaddr, uip_hostaddr);
      BUF->opcode = HTONS(ARP_REQUEST); /* ARP request. */
      BUF->hwtype = HTONS(ARP_HWTYPE_ETH);
      BUF->protocol = HTONS(UIP_ETHTYPE_IP);
      BUF->hwlen = 6;
      BUF->protolen = 4;
      BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);

      /* FIXME uip_appdata = &uip_buf[UIP_TCPIP_HLEN + UIP_LLH_LEN]; */
    
      uip_len = sizeof(struct arp_hdr);
      return 1;
    }

    /* Build an ethernet header. */
    memcpy(IPBUF->ethhdr.dest.addr, tabptr->ethaddr.addr, 6);
  }
  memcpy(IPBUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
  
  IPBUF->ethhdr.type = HTONS(UIP_ETHTYPE_IP);

  uip_len += sizeof(struct uip_eth_hdr);

  return 0;
}
/*-----------------------------------------------------------------------------------*/

/** @} */
/** @} */

struct arp_entry *
uip_arp_lookup (uip_ipaddr_t ipaddr)
{
  u8_t i;

  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    struct arp_entry *tabptr = &arp_table[i];
    if(uip_ipaddr_cmp(ipaddr, tabptr->ipaddr))
      return tabptr;
  }

  return NULL;
}

/*
  -- Ethersex META --
  header(protocols/uip/uip_arp.h)
  ifdef(`conf_BOOTLOADER', `', `ifdef(`conf_TEENSY', `', `timer(500, `uip_arp_timer()')')')
*/
