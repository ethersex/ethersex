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
#include <avr/eeprom.h>
#include "config.h"
#include "common.h"

#include "uip/uip.h"

/* for an eeprom memory map see doc/eeprom */

#define EEPROM_CONFIG_BASE  0x0000
#define EEPROM_CONFIG_EXT   0x0013

#define IPADDR_LEN sizeof(uip_ipaddr_t)

/* structures */

struct eeprom_config_base_t {
    uint8_t mac[6];

#ifndef UIP_CONF_IPV6
    /* IPv4 address to use, for IPv6 we use MAC-based autoconfiguration */
    uint8_t ip[IPADDR_LEN];
    uint8_t netmask[IPADDR_LEN];
    uint8_t gateway[IPADDR_LEN];
#endif /* not UIP_CONF_IPV6 */

    uint8_t crc;
};

struct eeprom_config_ext_t {
    uint8_t sntp_server[IPADDR_LEN];
    uint8_t syslog_server[IPADDR_LEN];

    uint8_t crc;
};

#endif
