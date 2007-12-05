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

#include <stdlib.h>
#include <string.h>
#include <util/crc16.h>
#include "eeprom.h"

uint8_t crc_checksum(void *data, uint8_t length)
/* {{{ */ {

    uint8_t crc = 0;
    uint8_t *p = (uint8_t *)data;

    for (uint8_t i = 0; i < length; i++) {
        crc = _crc_ibutton_update(crc, *p);
        p++;
    }

    return crc;

} /* }}} */


#if defined(ECMD_SUPPORT) || (defined(BOOTP_SUPPORT) \
			      && defined(BOOTP_TO_EEPROM_SUPPORT))
int8_t eeprom_save_config(void *mac, void *ip, void *netmask, void *gateway, void *dns_server)
/* {{{ */ {

    /* save new ip addresses */
    struct eeprom_config_base_t cfg_base;

    /* the eeprom section must contain valid data, if any parameter is NULL */
    eeprom_read_block(&cfg_base, EEPROM_CONFIG_BASE,
            sizeof(struct eeprom_config_base_t));

    if (mac != NULL)
        memcpy(&cfg_base.mac, mac, 6);
#if !UIP_CONF_IPV6 && (!defined(BOOTP_SUPPORT) \
                       || defined(BOOTP_TO_EEPROM_SUPPORT))
    if (ip != NULL)
        memcpy(&cfg_base.ip, ip, 4);
    if (netmask != NULL)
        memcpy(&cfg_base.netmask, netmask, 4);
    if (gateway != NULL)
        memcpy(&cfg_base.gateway, gateway, 4);
#endif /* not UIP_CONF_IPV6 and (not BOOTP or BOOTP_TO_EEPROM) */

#if defined(DNS_SUPPORT) && (!defined(BOOTP_SUPPORT) \
			     || defined(BOOTP_TO_EEPROM_SUPPORT))
    if (dns_server != NULL)
        memcpy(&cfg_base.dns_server, dns_server, IPADDR_LEN);
#endif

    /* calculate new checksum */
    uint8_t checksum = crc_checksum(&cfg_base, sizeof(struct eeprom_config_base_t) - 1);
    cfg_base.crc = checksum;

    /* save config */
    eeprom_write_block(&cfg_base, EEPROM_CONFIG_BASE,
            sizeof(struct eeprom_config_base_t));

    return 0;

} /* }}} */
#endif

