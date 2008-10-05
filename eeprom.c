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

#if defined(ECMD_PARSER_SUPPORT)  && ( ! defined(TEENSY_SUPPORT)) \
  || (defined(BOOTP_SUPPORT) && defined(BOOTP_TO_EEPROM_SUPPORT)) \
  || defined(ONEWIRE_SUPPORT) \
  || defined(TFTP_SUPPORT)
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
#endif /* ENC28J60_SUPPORT || ONEWIRE_SUPPORT */


#if defined(ECMD_PARSER_SUPPORT)  && ( ! defined(TEENSY_SUPPORT)) \
  || (defined(BOOTP_SUPPORT) && defined(BOOTP_TO_EEPROM_SUPPORT))
void 
eeprom_write_block_hack(void *dst, const void *src, size_t n)
{
  while (n--) {
    eeprom_write_byte(&((uint8_t *)dst)[n], ((uint8_t *)src)[n]);
  }
}

int8_t eeprom_save_config(void *mac, void *ip, void *netmask, void *gateway)
/* {{{ */ {
    (void) ip;
    (void) netmask;
    (void) gateway;

    /* save new ip addresses */
    struct eeprom_config_base_t cfg_base;

    /* the eeprom section must contain valid data, if any parameter is NULL */
    eeprom_read_block(&cfg_base, EEPROM_CONFIG_BASE,
            sizeof(struct eeprom_config_base_t));

    if (mac != NULL)
        memcpy(&cfg_base.mac, mac, 6);
#if (!UIP_CONF_IPV6 && (!defined(BOOTP_SUPPORT)			\
			|| defined(BOOTP_TO_EEPROM_SUPPORT)))	\
  || defined(OPENVPN_SUPPORT) || defined(IPV6_STATIC_SUPPORT)
    if (ip != NULL)
        memcpy(&cfg_base.ip, ip, IPADDR_LEN);
#endif

#if !UIP_CONF_IPV6 && (!defined(BOOTP_SUPPORT) \
                       || defined(BOOTP_TO_EEPROM_SUPPORT))
    if (netmask != NULL)
        memcpy(&cfg_base.netmask, netmask, 4);
    if (gateway != NULL)
        memcpy(&cfg_base.gateway, gateway, 4);
#endif /* not UIP_CONF_IPV6 and (not BOOTP or BOOTP_TO_EEPROM) */

    /* calculate new checksum */
    uint8_t checksum = crc_checksum(&cfg_base, sizeof(struct eeprom_config_base_t) - 1);
    cfg_base.crc = checksum;

    /* save config */
    eeprom_write_block_hack(EEPROM_CONFIG_BASE, &cfg_base,
            sizeof(struct eeprom_config_base_t));

    return 0;

} /* }}} */


int8_t eeprom_save_config_ext(struct eeprom_config_ext_t *new_cfg)
/* {{{ */ {
    /* save new ip addresses */
    struct eeprom_config_ext_t cfg_ext;

    /* the eeprom section must contain valid data, if any parameter is NULL */
    eeprom_read_block(&cfg_ext, EEPROM_CONFIG_EXT,
            sizeof(struct eeprom_config_ext_t));

#if defined(DNS_SUPPORT) && (!defined(BOOTP_SUPPORT) \
			     || defined(BOOTP_TO_EEPROM_SUPPORT))
    if (new_cfg->dns_server != NULL)
        memcpy(&cfg_ext.dns_server, new_cfg->dns_server, IPADDR_LEN);
#endif

#if defined(USART_SUPPORT)
    if (new_cfg->usart_baudrate != 0)
      cfg_ext.usart_baudrate = new_cfg->usart_baudrate;
#endif
#if defined(HTTPD_AUTH_SUPPORT)
    if (new_cfg->httpd_auth_password[0] != 0) {
      memcpy(cfg_ext.httpd_auth_password, new_cfg->httpd_auth_password, 
             sizeof(new_cfg->httpd_auth_password) - 1);
      cfg_ext.httpd_auth_password[sizeof(new_cfg->httpd_auth_password) - 1] = 0;
    }
#endif


    /* calculate new checksum */
    uint8_t checksum = crc_checksum(&cfg_ext, sizeof(struct eeprom_config_ext_t) - 1);
    cfg_ext.crc = checksum;

    /* save config */
    eeprom_write_block_hack(EEPROM_CONFIG_EXT, &cfg_ext,
            sizeof(struct eeprom_config_ext_t));

    return 0;

} /* }}} */
#endif

