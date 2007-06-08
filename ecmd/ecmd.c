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

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "../debug.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "ecmd.h"
#include "parser.h"


/* module local prototypes */
static int8_t parse_ip(char *cmd, uint8_t *ptr);
static int16_t parse_show(char *cmd, char *output, uint16_t len);

int16_t ecmd_parse_command(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD
    debug_printf("called ecmd_parse_command\n");
#endif

    int ret = -1;

    if (strncasecmp_P(cmd, PSTR("ip "), 3) == 0) {
        cmd += 2;

        /* allocate space for ip, netmask and gateway */
        uip_ipaddr_t *ips = malloc(sizeof(uip_ipaddr_t) * 3);

        for (uint8_t i = 0; i < 3; i++) {

            if (cmd == NULL) {
                ret = -1;
                break;
            }

            cmd += 1;

            /* try to parse ip */
            ret = parse_ip(cmd, (uint8_t *)&ips[i]);

            /* locate next whitespace char */
            cmd = strchr(cmd, ' ');

#ifdef DEBUG_ECMD_IP
            debug_printf("next string is '%s', ret is %d\n", cmd, ret);
#endif

            if (ret < 0) {
                ret = -1;
                break;
            }

#ifdef DEBUG_ECMD_IP
            debug_printf("successfully parsed ip param %u\n", i);
#endif
        }

        if (ret >= 0) {
#ifdef DEBUG_ECMD
            debug_printf("saving new network configuration\n");
#endif

            /* save new ip addresses, use uip_buf since this buffer is unused when
             * this function is executed */
            void *buf = uip_buf;

            /* the eeprom section must contain valid data, as the mac address is
             * not initialized here, but this is assured in network_init() */
            eeprom_read_block(buf, EEPROM_CONFIG_BASE, sizeof(struct eeprom_config_base_t));
            struct eeprom_config_base_t *cfg_base = (struct eeprom_config_base_t *)buf;

            memcpy(&cfg_base->ip, ips[0], sizeof(uip_ipaddr_t));
            memcpy(&cfg_base->netmask, ips[1], sizeof(uip_ipaddr_t));
            memcpy(&cfg_base->gateway, ips[2], sizeof(uip_ipaddr_t));

            /* calculate new checksum */
            uint8_t checksum = crc_checksum(buf, sizeof(struct eeprom_config_base_t) - 1);
            cfg_base->crc = checksum;

            /* save config */
            eeprom_write_block(buf, EEPROM_CONFIG_BASE, sizeof(struct eeprom_config_base_t));

            ret = 0;
        }

        /* free allocated space */
        free(ips);

    } else if (strncasecmp_P(cmd, PSTR("show "), 5) == 0) {
        cmd += 5;

        ret = parse_show(cmd, output, len);
    }

    if (ret == -1 && output != NULL) {
        strncpy_P(output, PSTR("parse error"), len);
        ret = 12;
    }

    return ret;
} /* }}} */

/* parse an ip address at cmd, write result to ptr */
int8_t parse_ip(char *cmd, uint8_t *ptr)
/* {{{ */ {

#ifdef DEBUG_ECMD_IP
    debug_printf("called parse_ip with string '%s'\n", cmd);
#endif

    int *ip = malloc(sizeof(int) * 4);

    /* return -2 if malloc() failed */
    if (ip == NULL)
        return -2;

    int ret = sscanf_P(cmd, PSTR("%u.%u.%u.%u"), ip, ip+1, ip+2, ip+3);

#ifdef DEBUG_ECMD_IP
    debug_printf("scanf returned %d\n", ret);
#endif

    if (ret == 4) {
#ifdef DEBUG_ECMD_IP
        debug_printf("read ip %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
#endif

        /* copy ip to destination */
        if (ptr != NULL)
            for (uint8_t i = 0; i < 4; i++)
                ptr[i] = ip[i];

        ret = 0;
    } else
        ret = -1;

    free(ip);
    return ret;
} /* }}} */

int16_t parse_show(char *cmd, char *output, uint16_t len)
/* {{{ */ {

    if (strncasecmp_P(cmd, PSTR("ip"), 2) == 0) {
        return snprintf_P(output, len, PSTR("ip %u.%u.%u.%u/%u.%u.%u.%u, gateway %u.%u.%u.%u"),
                LO8(uip_hostaddr[0]), HI8(uip_hostaddr[0]), LO8(uip_hostaddr[1]), HI8(uip_hostaddr[1]),
                LO8(uip_netmask[0]), HI8(uip_netmask[0]), LO8(uip_netmask[1]), HI8(uip_netmask[1]),
                LO8(uip_draddr[0]), HI8(uip_draddr[0]), LO8(uip_draddr[1]), HI8(uip_draddr[1]));
    }

    return -1;
} /* }}} */
