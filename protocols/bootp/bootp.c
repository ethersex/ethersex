/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

/* we want to live on the outer stack, if there are two ... */
#include "protocols/uip/uip_openvpn.h"

#include <avr/pgmspace.h>

#include <stdlib.h>
#include <string.h>

#include "protocols/uip/uip.h"
#include "core/eeprom.h"
#include "services/tftp/tftp.h"
#include "services/ntp/ntp.h"
#include "services/dyndns/dyndns.h"
#include "protocols/dns/resolv.h"

#include "bootp.h"
#include "bootphdr.h"
#include "bootp_net.h"
#include "core/debug.h"

static unsigned char mincookie[] = { 99, 130, 83, 99, 255 };
static unsigned char replycookie[] = { 0x63, 0x82, 0x53, 0x63 };



/*
 * send bootp request to broadcast address
 */
void
bootp_send_request(void)
{
  int i;

  /* prepare bootp request packet */
  struct bootp *pk = uip_appdata;
  memset(pk, 0, sizeof(struct bootp));

  pk->bp_op = BOOTREQUEST;
  pk->bp_htype = HTYPE_ETHERNET;
  pk->bp_hlen = 6;
  for (i = 0; i < 4; i++)
  {
    unsigned char c = rand() & 0xFF;    /* use upper bits ?? */

    pk->bp_xid[i] = c;
    uip_udp_conn->appstate.bootp.xid[i] = c;
  }

  pk->bp_flags |= HTONS(BPFLAG_BROADCAST);      /* request broadcast reply */
  memcpy(pk->bp_chaddr, uip_ethaddr.addr, 6);

  /* fill cookie into vendor data field */
  memcpy(pk->bp_vend, mincookie, sizeof(mincookie));

  /* broadcast the packet */
  uip_udp_send(sizeof(struct bootp));
}


void
bootp_handle_reply(void)
{
  int i;
  struct bootp *pk = uip_appdata;

  if (pk->bp_op != BOOTREPLY)
    return;                     /* ugh? shouldn't happen */

  if (pk->bp_htype != HTYPE_ETHERNET)
    return;

  for (i = 0; i < 4; i++)
  {
    if (pk->bp_xid[i] != uip_udp_conn->appstate.bootp.xid[i])
      return;                   /* session id doesn't match */

    if (pk->bp_vend[i] != replycookie[i])
      return;                   /* reply cookie doesn't match */
  }


  /*
   * looks like we have received a valid bootp reply,
   * prepare to override eeprom configuration
   */
  uip_ipaddr_t ips[5];
  memset(&ips, 0, sizeof(ips));

  /* extract our ip addresses, subnet-mask and gateway ... */
  memcpy(&ips[0], pk->bp_yiaddr, 4);
  uip_sethostaddr(&ips[0]);

  debug_printf("BOOTP: configured new ip address %d.%d.%d.%d\n",
               ((unsigned char *) ips)[0], ((unsigned char *) ips)[1],
               ((unsigned char *) ips)[2], ((unsigned char *) ips)[3]);

  unsigned char *ptr = pk->bp_vend + 4;
  while (*ptr != 0xFF)
  {
    switch (*ptr)
    {
      case TAG_SUBNET_MASK:
        memcpy(&ips[1], &ptr[2], 4);
        uip_setnetmask(&ips[1]);
        break;

      case TAG_GATEWAY:
        memcpy(&ips[2], &ptr[2], 4);
        uip_setdraddr(&ips[2]);
        break;
#ifdef DNS_SUPPORT
      case TAG_DOMAIN_SERVER:
        memcpy(&ips[3], &ptr[2], 4);
        resolv_conf(&ips[3]);
        break;
#endif
#ifdef NTP_SUPPORT
      case TAG_NTP_SERVER:
        /* This will set the ntp connection to the server set by the bootp
         * request
         */
        memcpy(&ips[4], &ptr[2], 4);
        ntp_conf(&ips[4]);
        break;
#endif
    }

    ptr = ptr + ptr[1] + 2;
  }

  /* Remove the bootp connection */
  uip_udp_remove(uip_udp_conn);

#ifdef BOOTP_TO_EEPROM_SUPPORT
  eeprom_save(ip, &ips[0], IPADDR_LEN);
  eeprom_save(netmask, &ips[1], IPADDR_LEN);
  eeprom_save(gateway, &ips[2], IPADDR_LEN);
#ifdef DNS_SUPPORT
  eeprom_save(dns_server, &ips[3], IPADDR_LEN);
#endif
#ifdef NTP_SUPPORT
  eeprom_save(ntp_server, &ips[4], IPADDR_LEN);
#endif
  eeprom_update_chksum();
#endif /* BOOTP_TO_EEPROM_SUPPORT */

#ifdef DYNDNS_SUPPORT
  dyndns_update();
#endif

#if defined(TFTP_SUPPORT) && defined(BOOTLOADER_SUPPORT)
  if (pk->bp_file[0] == 0)
    return;                     /* no boot filename provided */

  debug_putstr("load:");
  debug_putstr(pk->bp_file);
  debug_putchar('\n');

  /* create tftp connection, which will fire the download request */
  uip_ipaddr_t ip;
  uip_ipaddr(&ip, pk->bp_siaddr[0], pk->bp_siaddr[1],
             pk->bp_siaddr[2], pk->bp_siaddr[3]);

  tftp_fire_tftpomatic(&ip, pk->bp_file, 1);
#endif /* TFTP_SUPPORT */
}
