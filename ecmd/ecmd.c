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
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "ecmd.h"
#include "parser.h"


/* module local prototypes */
static int8_t parse_ip(char *cmd, uint8_t *ptr);
static int8_t parse_mac(char *cmd, uint8_t *ptr);
static int16_t parse_show(char *cmd, char *output, uint16_t len);

int16_t ecmd_parse_command(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD
    debug_printf("called ecmd_parse_command\n");
#endif

    int ret = -1;

    if (strncasecmp_P(cmd, PSTR("ip "), 3) == 0)
    /* {{{ */ {
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
            if ( (ret = eeprom_save_config(NULL, ips[0], ips[1], ips[2])) < 0)
                debug_printf("malloc failed!\n");
        }

        /* free allocated space */
        free(ips);

    } /* }}} */
    else if (strncasecmp_P(cmd, PSTR("mac "), 4) == 0)
    /* {{{ */ {
        cmd += 4;

        /* allocate space for mac */
        void *mac = malloc(sizeof(struct uip_eth_addr));

        if (mac == NULL) {
            debug_printf("malloc failed!\n");
            ret = -2;
        } else {
            ret = parse_mac(cmd, mac);
            eeprom_save_config(mac, NULL, NULL, NULL);
            free(mac);
        }

    } /* }}} */
    else if (strncasecmp_P(cmd, PSTR("show "), 5) == 0) {
        cmd += 5;
        ret = parse_show(cmd, output, len);
    } else if (strncasecmp_P(cmd, PSTR("reset"), 5) == 0) {
        void (*reset)(void) = (void *)0x0000;
        reset();
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

/* parse an ethernet address at cmd, write result to ptr */
int8_t parse_mac(char *cmd, uint8_t *ptr)
/* {{{ */ {

#ifdef DEBUG_ECMD_MAC
    debug_printf("called parse_mac with string '%s'\n", cmd);
#endif

    int *mac = malloc(sizeof(int) * 6);

    /* return -2 if malloc() failed */
    if (mac == NULL)
        return -2;

    int ret = sscanf_P(cmd, PSTR("%x:%x:%x:%x:%x:%x"), mac, mac+1, mac+2, mac+3, mac+4, mac+5);

#ifdef DEBUG_ECMD_MAC
    debug_printf("scanf returned %d\n", ret);
#endif

    if (ret == 6) {
#ifdef DEBUG_ECMD_MAC
        debug_printf("read mac %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

        /* copy mac to destination */
        if (ptr != NULL)
            for (uint8_t i = 0; i < 6; i++)
                ptr[i] = mac[i];

        ret = 0;
    } else
        ret = -1;

    free(mac);
    return ret;
} /* }}} */

int16_t parse_show(char *cmd, char *output, uint16_t len)
/* {{{ */ {

    if (strncasecmp_P(cmd, PSTR("ip"), 2) == 0) {
        uint8_t *ips = malloc(sizeof(uip_ipaddr_t)*3);

        eeprom_read_block(ips, EEPROM_IPS_OFFSET, sizeof(uip_ipaddr_t)*3);

        int output_len = snprintf_P(output, len, PSTR("ip %u.%u.%u.%u/%u.%u.%u.%u, gateway %u.%u.%u.%u"),
                ips[0], ips[1], ips[2], ips[3],
                ips[4], ips[5], ips[6], ips[7],
                ips[8], ips[9], ips[10], ips[11]);

        free(ips);
        return output_len;
    } else if (strncasecmp_P(cmd, PSTR("mac"), 3) == 0) {
        uint8_t *mac = malloc(sizeof(struct uip_eth_addr));

        eeprom_read_block(mac, EEPROM_MAC_OFFSET, sizeof(struct uip_eth_addr));

        return snprintf_P(output, len, PSTR("mac %02x:%02x:%02x:%02x:%02x:%02x"),
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        free(mac);
    }

    return -1;
} /* }}} */
