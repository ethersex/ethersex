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

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../fs20/fs20.h"
#include "../portio.h"
#include "ecmd.h"


/* module local prototypes */
/* high level */
static int16_t parse_cmd_ip(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_mac(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_show_ip(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_show_mac(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_reset(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_io_set_ddr(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_io_get_ddr(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_io_set_port(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_io_get_port(char *cmd, char *output, uint16_t len);
static int16_t parse_cmd_io_get_pin(char *cmd, char *output, uint16_t len);
#ifdef FS20_SUPPORT_SEND
static int16_t parse_cmd_send_fs20(char *cmd, char *output, uint16_t len);
#endif
#ifdef FS20_SUPPORT_RECEIVE
static int16_t parse_cmd_recv_fs20(char *cmd, char *output, uint16_t len);
#endif

/* low level */
static int8_t parse_ip(char *cmd, uint8_t *ptr);
static int8_t parse_mac(char *cmd, uint8_t *ptr);

/* struct for storing commands */
struct ecmd_command_t {
    PGM_P name;
    int16_t (*func)(char*, char*, uint16_t);
};

/* construct strings.  this is ugly, but the only known way of
 * storing structs containing string pointer completely in program
 * space */
const char PROGMEM ecmd_showmac_text[] = "show mac";
const char PROGMEM ecmd_showip_text[] = "show ip";
const char PROGMEM ecmd_ip_text[] = "ip ";
const char PROGMEM ecmd_mac_text[] = "mac ";
const char PROGMEM ecmd_reset_text[] = "reset";
const char PROGMEM ecmd_io_set_ddr[] = "io set ddr";
const char PROGMEM ecmd_io_get_ddr[] = "io get ddr";
const char PROGMEM ecmd_io_set_port[] = "io set port";
const char PROGMEM ecmd_io_get_port[] = "io get port";
const char PROGMEM ecmd_io_get_pin[] = "io get pin";
#ifdef FS20_SUPPORT_SEND
const char PROGMEM ecmd_fs20_send_text[] = "fs20 send";
#endif
#ifdef FS20_SUPPORT_RECEIVE
const char PROGMEM ecmd_fs20_recv_text[] = "fs20 receive";
#endif

const struct ecmd_command_t PROGMEM ecmd_cmds[] = {
    { ecmd_ip_text, parse_cmd_ip },
    { ecmd_showmac_text, parse_cmd_show_mac },
    { ecmd_showip_text, parse_cmd_show_ip },
    { ecmd_mac_text, parse_cmd_mac },
    { ecmd_reset_text, parse_cmd_reset },
    { ecmd_io_set_ddr, parse_cmd_io_set_ddr },
    { ecmd_io_get_ddr, parse_cmd_io_get_ddr },
    { ecmd_io_set_port, parse_cmd_io_set_port },
    { ecmd_io_get_port, parse_cmd_io_get_port },
    { ecmd_io_get_pin, parse_cmd_io_get_pin },
#ifdef FS20_SUPPORT_SEND
    { ecmd_fs20_send_text, parse_cmd_send_fs20 },
#endif
#ifdef FS20_SUPPORT_RECEIVE
    { ecmd_fs20_recv_text, parse_cmd_recv_fs20 },
#endif
    { NULL, NULL },
};

int16_t ecmd_parse_command(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD
    debug_printf("called ecmd_parse_command\n");
#endif

    if (strlen(cmd) <= 2) {
#ifdef DEBUG_ECMD
        debug_printf("cmd is too short\n");
#endif
        return 0;
    }

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

int16_t parse_cmd_show_mac(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_MAC
    debug_printf("called parse_cmd_show with rest: \"%s\"\n", cmd);
#endif

    struct uip_eth_addr buf;
    uint8_t *mac = (uint8_t *)&buf;

    eeprom_read_block(&buf, EEPROM_MAC_OFFSET, sizeof(struct uip_eth_addr));

    int output_len = snprintf_P(output, len,
            PSTR("mac %02x:%02x:%02x:%02x:%02x:%02x"),
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return output_len;
} /* }}} */

int16_t parse_cmd_show_ip(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    uint8_t ips[sizeof(uip_ipaddr_t)*3];

    eeprom_read_block(ips, EEPROM_IPS_OFFSET, sizeof(uip_ipaddr_t)*3);

    int output_len = snprintf_P(output, len,
            PSTR("ip %u.%u.%u.%u/%u.%u.%u.%u, gateway %u.%u.%u.%u"),
            ips[0], ips[1], ips[2], ips[3],
            ips[4], ips[5], ips[6], ips[7],
            ips[8], ips[9], ips[10], ips[11]);

    return output_len;
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

#ifdef FS20_SUPPORT_SEND
static int16_t parse_cmd_send_fs20(char *cmd, char *output, uint16_t len)
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
#endif

#ifdef FS20_SUPPORT_RECEIVE
static int16_t parse_cmd_recv_fs20(char *cmd, char *output, uint16_t len)
/* {{{ */ {

    char *s = output;
    uint8_t l = 0;
    uint8_t outlen = 0;

#ifdef DEBUG_ECMD_FS20
    debug_printf("%u positions in queue\n", fs20_global.len);
#endif

    while (l < fs20_global.len &&
            (uint8_t)(outlen+9) < len) {
#ifdef DEBUG_ECMD_FS20
        debug_printf("generating for pos %u: %02x%02x%02x%02x", l,
                fs20_global.queue[l].hc1,
                fs20_global.queue[l].hc2,
                fs20_global.queue[l].addr,
                fs20_global.queue[l].cmd);
#endif

        sprintf_P(s, PSTR("%02x%02x%02x%02x\n"),
                fs20_global.queue[l].hc1,
                fs20_global.queue[l].hc2,
                fs20_global.queue[l].addr,
                fs20_global.queue[l].cmd);

        s += 9;
        outlen += 9;
        l++;

#ifdef DEBUG_ECMD_FS20
        *s = '\0';
        debug_printf("output is \"%s\"\n", output);
#endif
    }

    /* clear queue */
    fs20_global.len = 0;

    return outlen;

} /* }}} */
#endif

static int16_t parse_cmd_io_set_ddr(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_set_ddr with rest: \"%s\"\n", cmd);
#endif

    uint16_t port, data, mask;

    int ret = sscanf_P(cmd,
            PSTR("%x %x %x"),
            &port, &data, &mask);

    /* use default mask, if no mask has been given */
    if (ret == 2) {
        mask = 0xff;
        ret = 3;
    }

    if (ret == 3 && port < IO_PORTS) {

        cfg.options.io_ddr[port] = (cfg.options.io_ddr[port] & ~mask) |
                                   LO8(data & mask);

        return 0;
    } else
        return -1;

} /* }}} */

static int16_t parse_cmd_io_get_ddr(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_ddr with rest: \"%s\"\n", cmd);
#endif

    uint16_t port;

    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);

    if (ret == 1 && port < IO_PORTS) {

        return snprintf_P(output, len,
                PSTR("port %d: 0x%02x"),
                port,
                cfg.options.io_ddr[port]);
    } else
        return -1;

} /* }}} */

static int16_t parse_cmd_io_set_port(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_set_port with rest: \"%s\"\n", cmd);
#endif

    uint16_t port, data, mask;

    int ret = sscanf_P(cmd,
            PSTR("%x %x %x"),
            &port, &data, &mask);

    /* use default mask, if no mask has been given */
    if (ret == 2) {
        mask = 0xff;
        ret = 3;
    }

    if (ret == 3 && port < IO_PORTS) {

        cfg.options.io[port] = (cfg.options.io[port] & ~mask) |
                                   LO8(data & mask);

        return 0;
    } else
        return -1;

} /* }}} */

static int16_t parse_cmd_io_get_port(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_port with rest: \"%s\"\n", cmd);
#endif

    uint16_t port;

    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);

    if (ret == 1 && port < IO_PORTS) {

        return snprintf_P(output, len,
                PSTR("port %d: 0x%02x"),
                port,
                cfg.options.io[port]);
    } else
        return -1;

} /* }}} */

static int16_t parse_cmd_io_get_pin(char *cmd, char *output, uint16_t len)
/* {{{ */ {

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_pin with rest: \"%s\"\n", cmd);
#endif

    uint16_t port;

    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);

    if (ret == 1 && port < IO_PORTS) {

        return snprintf_P(output, len,
                PSTR("port %d: 0x%02x"),
                port,
                portio_input(port));
    } else
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
