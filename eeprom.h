/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _EEPROM_H
#define _EEPROM_H

#include <stdint.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include "config.h"
#include "global.h"

/* for an eeprom memory map see doc/eeprom */

#define IPADDR_LEN sizeof(uip_ipaddr_t)

/* structures */

struct eeprom_config_base_t {
    uint8_t mac[6];

    /* IPv4 address to use, for IPv6 we use MAC-based autoconfiguration */
#if (!UIP_CONF_IPV6 && (!defined(BOOTP_SUPPORT)			\
			|| defined(BOOTP_TO_EEPROM_SUPPORT)))	\
  || defined(OPENVPN_SUPPORT) || (UIP_CONF_IPV6 && !defined(ENC28J60_SUPPORT))
    uint8_t ip[IPADDR_LEN];
#endif

#if !UIP_CONF_IPV6 && (!defined(BOOTP_SUPPORT) \
                       || defined(BOOTP_TO_EEPROM_SUPPORT))
    uint8_t netmask[IPADDR_LEN];
    uint8_t gateway[IPADDR_LEN];
#endif /* not UIP_CONF_IPV6 and (not BOOTP or BOOTP_TO_EEPROM) */

    uint8_t crc;
};

struct eeprom_config_ext_t {
#if defined(DNS_SUPPORT) && (!defined(BOOTP_SUPPORT) \
			     || defined(BOOTP_TO_EEPROM_SUPPORT))
    uint8_t dns_server[IPADDR_LEN];
#endif
#if defined(USART_SUPPORT)
    uint16_t usart_baudrate;
#endif

    uint8_t crc;
};

#define EEPROM_CONFIG_BASE  (uint8_t *)0x0000
#define EEPROM_CONFIG_EXT   (uint8_t *) sizeof(struct eeprom_config_base_t)

#define EEPROM_MAC_OFFSET   ((EEPROM_CONFIG_BASE) + \
    offsetof(struct eeprom_config_base_t, mac[0]))
#define EEPROM_IPS_OFFSET   ((EEPROM_CONFIG_BASE) + \
    offsetof(struct eeprom_config_base_t, ip[0]))

uint8_t crc_checksum(void *data, uint8_t length);
int8_t eeprom_save_config(void *mac, void *ip, void *netmask, void *gateway);
int8_t eeprom_save_config_ext(struct eeprom_config_ext_t *new_cfg);

#endif
