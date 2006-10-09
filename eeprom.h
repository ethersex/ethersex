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

/* structures */
struct eeprom_config_t {
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
    uint8_t sntp_server[4];
    uint8_t syslog_server[4];
    uint8_t crc;
};

#if 0
static struct eeprom_config_t eeprom_config EEMEM = {
    { 0xac, 0xde, 0x48, 0xfd, 0x0f, 0xd1 },
    { 137, 226, 146, 59 },
    { 255, 255, 254, 0 },
    { 137, 226, 147, 1 },
    { 134, 130, 4, 17 },
    { 137, 226, 147, 211},
    0x13,
};
#elif 1
static struct eeprom_config_t eeprom_config EEMEM = {
    { 0xac, 0xde, 0x48, 0xfd, 0x0f, 0xd1 },
    { 10, 0, 0, 2 },
    { 255, 255, 255, 0 },
    { 10, 0, 0, 1 },
    { 10, 0, 0, 1 },
    { 0, 0, 0, 0 },
    0x55,
};
#else
static struct eeprom_config_t eeprom_config EEMEM = {
    { 0xac, 0xde, 0x48, 0xfd, 0x0f, 0xd1 },
    { 172, 23, 23, 52 },
    { 255, 255, 255, 0 },
    { 172, 23, 23, 1 },
    { 172, 23, 23, 1 },
    { 172, 23, 23, 1 },
    0x5B,
};
#endif

/* prototypes */
void eeprom_load_ip(uint8_t *addr, uip_ipaddr_t *dest);

#endif
