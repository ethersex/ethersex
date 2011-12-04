/* 
 * Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "core/debug.h"
#include "core/host/tap.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/uip/uip_arp.h"

int tap_fd;

#define die(a...) do { fprintf(stderr, a); exit (-1); } while(0)

int 
tap_alloc(char *dev)
{
  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
    return -1;

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
   *        IFF_TAP   - TAP device  
   *
   *        IFF_NO_PI - Do not provide packet information  
   */ 
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI; 
  if( *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}              

#define exec_cmd(a...)						\
  do {								\
    char *cmd = g_strdup_printf(a);				\
    int ret = system(cmd);					\
    if (ret != 0)						\
      die("Setting IP address failed with %d (%s)", ret, cmd);	\
    g_free(cmd);						\
  } while(0)

void 
open_tap(void)
{
  char tap_name[16] = "esex%d";

  tap_fd = tap_alloc(tap_name);
  if (tap_fd < 0)
    die("Couldn't open tap device");


#ifdef IPV6_SUPPORT
  exec_cmd("ip -6 a a " CONF_TAP_LOCALIP "/%d dev %s", CONF_TAP_LOCALPLEN, tap_name);
  exec_cmd("ip link set %s up", tap_name);
#else
  exec_cmd("ifconfig %s " CONF_TAP_LOCALIP " netmask " CONF_TAP_IP4_NETMASK " up", tap_name);
#endif
}


void
tap_read (void)
{
  uip_stack_set_active(STACK_TAP);
  uip_len = read (tap_fd, uip_buf, UIP_CONF_BUFFER_SIZE);

  /* process packet */
  struct uip_eth_hdr *packet = (struct uip_eth_hdr *)&uip_buf;

#ifdef IEEE8021Q_SUPPORT
  /* Check VLAN tag. */
  if (packet->tpid != HTONS(0x8100)
      || (packet->vid_hi & 0x1F) != (CONF_8021Q_VID >> 8)
      || packet->vid_lo != (CONF_8021Q_VID & 0xFF)) {
    debug_printf("net: wrong vlan tag detected.\n");
  }
  else
#endif
    switch (HTONS(packet->type)) {

#if !UIP_CONF_IPV6
      /* process arp packet */
    case UIP_ETHTYPE_ARP:
#ifdef DEBUG_NET
      debug_printf("net: arp packet received\n");
#endif
      uip_arp_arpin();

      /* if there is a packet to send, send it now */
      if (uip_len > 0)
	tap_send();

      break;
#endif /* !UIP_CONF_IPV6 */

#if UIP_CONF_IPV6
      /* process ip packet */
    case UIP_ETHTYPE_IP6:
#ifdef DEBUG_NET
      debug_printf ("net: ip6 packet received\n");
#endif
#else  /* !UIP_CONF_IPV6 */
      /* process ip packet */
    case UIP_ETHTYPE_IP:
#ifdef DEBUG_NET
      debug_printf ("net: ip packet received\n");
#endif
      uip_arp_ipin();
#endif	/* !UIP_CONF_IPV6 */

      router_input(STACK_TAP);

      /* if there is a packet to send, send it now */
      if (uip_len > 0)
	router_output();

      break;
    }
}


void
tap_send (void)
{
#ifdef IEEE8021Q_SUPPORT
  /* Write VLAN-tag to outgoing packet. */
  struct uip_eth_hdr *eh = (struct uip_eth_hdr *) uip_buf;
  eh->tpid = HTONS(0x8100);
  eh->vid_hi = (CONF_8021Q_VID >> 8) | (CONF_8021Q_PRIO << 5);
  eh->vid_lo = CONF_8021Q_VID & 0xFF;
#endif

  write (tap_fd, uip_buf, uip_len);
}


uint8_t
tap_txstart(void)
{
  uint8_t retval;

#if UIP_CONF_IPV6
  retval = uip_neighbor_out();
  if (uip_len)
#else
  retval = uip_arp_out();
#endif
  tap_send();

  return retval;
}



/*
  -- Ethersex META --
  header(core/host/tap.h)
  init(open_tap)
*/
