/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright(c) 2008 Stefan Siegl <stesie@brokenpipe.de>
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
#include <stddef.h>
#include <string.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "eeprom.h"


#ifdef CRC_SUPPORT
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
#endif /* CRC_SUPPORT */


#ifdef EEPROM_SUPPORT

void
eeprom_write_block_hack(void *dst, const void *src, size_t n)
{
  while (n--) {
    eeprom_write_byte(&((uint8_t *)dst)[n], ((uint8_t *)src)[n]);
  }
}


uint8_t
eeprom_get_chksum(void) 
{
    uint8_t eeprom_crc = 0;
    uint8_t *p = (uint8_t *)EEPROM_CONFIG_BASE;

    for (uint8_t i = 0; i < (sizeof(struct eeprom_config_t) - 1 ); i++) {
        eeprom_crc = _crc_ibutton_update(eeprom_crc, eeprom_read_byte(p));
        p++;
    }

    return eeprom_crc;
}



void
eeprom_init (void)
{
    uip_ipaddr_t ip;
    (void) ip;			/* Keep GCC quiet. */

    eeprom_save_P (mac, PSTR(CONF_ETHERRAPE_MAC), 6);

#if (defined(IPV4_SUPPORT) && !defined(BOOTP_SUPPORT)) || defined(IPV6_STATIC_SUPPORT)
    eeprom_save_ip (ip, CONF_ETHERRAPE_IP);
#endif

#ifdef IPV4_SUPPORT
    eeprom_save_ip (netmask, CONF_ETHERRAPE_IP4_NETMASK);
    eeprom_save_ip (gateway, CONF_ETHERRAPE_IP4_GATEWAY);
#endif

#ifdef DNS_SUPPORT
    eeprom_save_ip (dns_server, CONF_DNS_SERVER);
#endif

#ifdef YPORT_SUPPORT		/* FIXME 2nd usart? */
    eeprom_save_int (usart_baudrate, YPORT_BAUDRATE);
#elif defined(ZBUS_SUPPORT)
    eeprom_save_int (usart_baudrate, CONF_ZBUS_BAUDRATE/100);
#endif

#ifdef HTTPD_AUTH_SUPPORT
    /* Copy the httpd's password. */
    eeprom_save_P (httpd_auth_password, PSTR(CONF_HTTPD_PASSWORD), 8);
    eeprom_save_char (httpd_auth_null_byte, 0);
#endif

    eeprom_update_chksum();
}


#endif	/* EEPROM_SUPPORT */

