/*
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

#include <avr/eeprom.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "network.h"
#include "config.h"
#include "core/eeprom.h"
#include "core/mbr.h"
#include "protocols/uip/ipv6.h"
#include "protocols/zbus/zbus.h"

#include "core/debug.h"

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_neighbor.h"
#include "protocols/uip/uip_rfm12.h"
#include "protocols/uip/uip_zbus.h"
#include "services/tftp/tftp.h"
#include "hardware/ethernet/enc28j60.h"

extern void ethersex_meta_netinit (void);

void
network_init(void)
{
    uip_ipaddr_t ip;
    (void) ip;			/* Keep GCC quiet. */

    uip_init();

#if defined(RFM12_IP_SUPPORT) && defined(UIP_MULTI_STACK)
    uip_stack_set_active(STACK_RFM12);
    rfm12_stack_init();
#endif

#if defined(ZBUS_SUPPORT) && defined(UIP_MULTI_STACK)
    uip_stack_set_active(STACK_ZBUS);
    zbus_stack_init();
#endif

#ifdef OPENVPN_SUPPORT
    uip_stack_set_active(STACK_OPENVPN);
    openvpn_init();
#endif

    /* load base network settings */
#   ifdef DEBUG_NET_CONFIG
    debug_printf("net: loading base network settings\n");
#   endif

#ifdef ETHERNET_SUPPORT
# ifdef ENC28J60_SUPPORT
    uip_stack_set_active(STACK_ENC);
# else  /* TAP_SUPPORT */
    uip_stack_set_active(STACK_TAP);
#endif

    /* use uip buffer as generic space here, since when this function is called,
     * no network packets will be processed */

#ifdef EEPROM_SUPPORT
    /* use global network packet buffer for configuration */
    uint8_t checksum = eeprom_get_chksum();
    uint8_t saved_checksum;
    eeprom_restore_char(crc, &saved_checksum);


    if (checksum != saved_checksum)
      eeprom_init();
#endif

#ifdef ETHERNET_SUPPORT
    network_config_load();
#endif



    /* Do the autoconfiguration after the MAC is set */
#   if UIP_CONF_IPV6 && !defined(IPV6_STATIC_SUPPORT)
    uip_setprefixlen(64);
    uip_ip6autoconfig(0xFE80, 0x0000, 0x0000, 0x0000);
#   endif

#   if defined(IPV6_STATIC_SUPPORT) && defined(TFTPOMATIC_SUPPORT)
    const char *filename = CONF_TFTP_IMAGE;
    set_CONF_TFTP_IP(&ip);
#ifdef MBR_SUPPORT
    if (mbr.bootloader == 1)
#endif
    {
      tftp_fire_tftpomatic(&ip, filename, 1);
      bootload_delay = CONF_BOOTLOAD_DELAY;
    }
#   endif /* IPV6_STATIC_SUPPORT && TFTPOMATIC_SUPPORT */


#   elif !defined(ROUTER_SUPPORT) /* and not ETHERNET_SUPPORT */
    /* Don't allow for eeprom-based configuration of rfm12/zbus IP address,
       mainly for code size reasons. */
    set_CONF_ETHERSEX_IP(&ip);
    uip_sethostaddr(&ip);

#   endif /* not ETHERNET_SUPPORT and not ROUTER_SUPPORT */

    ethersex_meta_netinit();

#   ifdef ENC28J60_SUPPORT
    init_enc28j60();
#   endif

#   ifdef ETHERNET_SUPPORT
#   if UIP_CONF_IPV6
    uip_neighbor_init();
#   else
    uip_arp_init();
#   endif
#   else /* ETHERNET_SUPPORT */
    /* set at least fixed default gateway address
     * to allow multi stack routing */
    set_CONF_ETHERSEX_GATEWAY(&ip);
    uip_setdraddr(&ip);
#   endif  /* ETHERNET_SUPPORT */

}

/*
  -- Ethersex META --
  header(network.h)
  init(network_init)
*/
