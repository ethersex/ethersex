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
#include "../bit-macros.h"
#include "../fs20.h"
#include "ecmd.h"
#include "parser.h"


/* module local prototypes */
/* high level */
static int16_t parse_cmd_ip(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_mac(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_show(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_reset(char *cmd, char *output, uint16_t len);
#ifdef FS20_SUPPORT
static int16_t parse_cmd_fs20(char *cmd, char *output, uint16_t len);
#endif

/* low level */
static int8_t parse_ip(char *cmd, uint8_t *ptr);
static int8_t parse_mac(char *cmd, uint8_t *ptr);

/* struct for storing commands */
struct ecmd_command_t {
    PGM_P name;
    int16_t (*func)(char*, char*, uint16_t);
};

const char PROGMEM ecmd_show_text[] = "show ";
const char PROGMEM ecmd_ip_text[] = "ip ";
const char PROGMEM ecmd_mac_text[] = "mac ";
const char PROGMEM ecmd_reset_text[] = "reset";
#ifdef FS20_SUPPORT
const char PROGMEM ecmd_fs20_text[] = "fs20 send";
#endif

const struct ecmd_command_t PROGMEM ecmd_cmds[] = {
    { ecmd_ip_text, parse_cmd_ip },
    { ecmd_show_text, parse_cmd_show },
    { ecmd_mac_text, parse_cmd_mac },
    { ecmd_reset_text, parse_cmd_reset },
#ifdef FS20_SUPPORT
    { ecmd_fs20_text, parse_cmd_fs20 },
#endif
    { NULL, NULL },
};

int16_t ecmd_parse_command(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD
    debug_printf("called ecmd_parse_command\n");
#endif

    int ret = -1;

    char *text = NULL;
    int16_t (*func)(char*, char*, uint16_t) = NULL;
    uint8_t pos = 0;

    while (1) {
        /* load pointer to text */
        text = (char *)pgm_read_word(&ecmd_cmds[pos].name);

#ifdef DEBUG_ECMD
        debug_printf("loaded text addres %p: \n", text);
#endif

        /* return if we reached the end of the array */
        if (text == NULL)
            break;

#ifdef DEBUG_ECMD
        debug_printf("text is: \"");
        printf_P(text);
        debug_printf("\"\n");
#endif

        /* else compare texts */
        if (strncasecmp_P(cmd, text, strlen_P(text)) == 0) {
#ifdef DEBUG_ECMD
            debug_printf("found match\n");
#endif
            cmd += strlen_P(text);
            func = (void *)pgm_read_word(&ecmd_cmds[pos].func);
            break;
        }

        pos++;
    }

#ifdef DEBUG_ECMD
    debug_printf("rest cmd: \"%s\"\n", cmd);
#endif

    if (func != NULL)
        ret = func(cmd, output, len);

    if (ret == -1 && output != NULL) {
        strncpy_P(output, PSTR("parse error"), len);
        ret = 12;
    } else if (ret == 0) {
        strncpy_P(output, PSTR("OK"), len);
        ret = 2;
    }

    return ret;
} /* }}} */

/* high level parsing functions */

int16_t parse_cmd_show(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_MAC
    debug_printf("called parse_cmd_show with rest: \"%s\"\n", cmd);
#endif

    if (strncasecmp_P(cmd, PSTR("ip"), 2) == 0) {
        uint8_t *ips = malloc(sizeof(uip_ipaddr_t)*3);

        eeprom_read_block(ips, EEPROM_IPS_OFFSET, sizeof(uip_ipaddr_t)*3);

        int output_len = snprintf_P(output, len,
                PSTR("ip %u.%u.%u.%u/%u.%u.%u.%u, gateway %u.%u.%u.%u"),
                ips[0], ips[1], ips[2], ips[3],
                ips[4], ips[5], ips[6], ips[7],
                ips[8], ips[9], ips[10], ips[11]);

        free(ips);
        return output_len;
    } else if (strncasecmp_P(cmd, PSTR("mac"), 3) == 0) {
        uint8_t *mac = malloc(sizeof(struct uip_eth_addr));

        eeprom_read_block(mac, EEPROM_MAC_OFFSET, sizeof(struct uip_eth_addr));

        int output_len = snprintf_P(output, len,
                PSTR("mac %02x:%02x:%02x:%02x:%02x:%02x"),
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        free(mac);
        return output_len;
    }

    return -1;
} /* }}} */

static int16_t parse_cmd_ip(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_IP
    debug_printf("called with string %s\n", cmd);
#endif

    int8_t ret;

    uip_ipaddr_t ips[3];

    for (uint8_t i = 0; i < 3; i++) {

        if (cmd == NULL)
            return -1;

        while (*cmd == ' ')
            cmd++;

        /* try to parse ip */
        ret = parse_ip(cmd, (uint8_t *)&ips[i]);

        /* locate next whitespace char */
        cmd = strchr(cmd, ' ');

#ifdef DEBUG_ECMD_IP
        debug_printf("next string is '%s', ret is %d\n", cmd, ret);
#endif

        if (ret < 0)
            return -1;

#ifdef DEBUG_ECMD_IP
        debug_printf("successfully parsed ip param %u\n", i);
#endif
    }

#ifdef DEBUG_ECMD
    debug_printf("saving new network configuration\n");
#endif

    /* save new ip addresses, use uip_buf since this buffer is unused when
     * this function is executed */
    return eeprom_save_config(NULL, ips[0], ips[1], ips[2]);

} /* }}} */

static int16_t parse_cmd_mac(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_MAC
    debug_printf("called with string %s\n", cmd);
#endif

    int8_t ret;

    /* allocate space for mac */
    struct uip_eth_addr mac;

    ret = parse_mac(cmd, (void *)&mac);

    if (ret >= 0)
        return eeprom_save_config(&mac, NULL, NULL, NULL);
    else
        return ret;

} /* }}} */

static int16_t parse_cmd_reset(char *cmd, char *output, uint16_t len)
/* {{{ */ {

    void (*reset)(void) = (void *)0x0000;
    reset();
    return 0;

} /* }}} */

static int16_t parse_cmd_fs20(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_FS20
    debug_printf("called with string %s\n", cmd);
#endif

    uint16_t hc, addr, c;

    int ret = sscanf_P(cmd,
            PSTR("%x %x %x"),
            &hc, &addr, &c);

    if (ret == 3) {
#ifdef DEBUG_ECMD_FS20
        debug_printf("fs20_send(0x%x,0x%x,0x%x)\n", hc, LO8(addr), LO8(c));
#endif

        fs20_send(hc, LO8(addr), LO8(c));
        return 0;
    }

    return -1;

} /* }}} */

/* low level parsing functions */

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
