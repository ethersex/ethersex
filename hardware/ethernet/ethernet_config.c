/*
 *
 *          enc28j60 api
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include "config.h"
#include <avr/pgmspace.h>
#include "network.h"
#include "protocols/uip/uip.h"
#include "core/eeprom.h"

static const char conf_mac[] PROGMEM = CONF_ETHERSEX_MAC;

void
network_config_load (void)
{
  /* load settings from eeprom */
#ifdef EEPROM_SUPPORT
  eeprom_restore (mac, uip_ethaddr.addr, 6);
#else
#if defined(BOOTLOADER_SUPPORT) &&  BOOTLOADER_START_ADDRESS > UINT16_MAX
  uint_farptr_t src = pgm_get_far_address (conf_mac);
  uint8_t *dst = uip_ethaddr.addr;
  for (uint8_t i = 6; i; i--)
    *dst++ = pgm_read_byte_far (src++);
#else
  memcpy_P (uip_ethaddr.addr, conf_mac, 6);
#endif
#endif

#if (defined(IPV4_SUPPORT) && !defined(BOOTP_SUPPORT) && !defined(DHCP_SUPPORT)) || defined(IPV6_STATIC_SUPPORT)
  uip_ipaddr_t ip;

  /* Configure the IP address. */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip (ip, &ip);
#else
  set_CONF_ETHERSEX_IP (&ip);
#endif
  uip_sethostaddr (&ip);

  /* Configure prefix length (IPv6). */
#ifdef IPV6_SUPPORT
  uip_setprefixlen (CONF_ENC_IP6_PREFIX_LEN);
#endif

#ifdef IPV4_SUPPORT
  /* Configure the netmask (IPv4). */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip (netmask, &ip);
#else
  set_CONF_ETHERSEX_IP4_NETMASK (&ip);
#endif
  uip_setnetmask (&ip);
#endif /* IPV4_SUPPORT */

  /* Configure the default gateway  */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip (gateway, &ip);
#else
  set_CONF_ETHERSEX_GATEWAY (&ip);
#endif
  uip_setdraddr (&ip);
#endif /* No autoconfiguration. */
}

#if defined(IPV6_SUPPORT) && !defined(IPV6_STATIC_SUPPORT)
void ethernet_config_periodic(void)
{
  static uint8_t counter = 0;

  if (counter == 0) {
    // Send a router solicitation every 10 seconds, as long
    // as we only got a link local address.  First time one
    // second after boot
    if(((u16_t *)(uip_hostaddr))[0] == HTONS(0xFE80)) {
      uip_router_send_solicitation();
      transmit_packet();
    }
  }

  counter++;
  if (counter == 10)
    counter = 0;
}
#endif

/*
  -- Ethersex META --
  header(hardware/ethernet/enc28j60.h)
  ifdef(`conf_IPV6', `ifdef(`conf_IPV6_STATIC', `', `timer(50, `ethernet_config_periodic()')')')
*/
