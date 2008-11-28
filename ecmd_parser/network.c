/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "ecmd.h"

#ifdef UIP_SUPPORT

#ifndef TEENSY_SUPPORT
int16_t print_ipaddr (uip_ipaddr_t *addr, char *output, uint16_t len) 
/* {{{ */ {
#if UIP_CONF_IPV6
  return snprintf_P (output, len, PSTR ("%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"),
		     HTONS(((u16_t *)(addr))[0]), HTONS(((u16_t *)(addr))[1]), 
		     HTONS(((u16_t *)(addr))[2]), HTONS(((u16_t *)(addr))[3]), 
		     HTONS(((u16_t *)(addr))[4]), HTONS(((u16_t *)(addr))[5]), 
		     HTONS(((u16_t *)(addr))[6]), HTONS(((u16_t *)(addr))[7]));
#else
  uint8_t *ip = (uint8_t *) addr;
  int output_len = snprintf_P (output, len, PSTR ("%u.%u.%u.%u"), 
			       ip[0], ip[1], ip[2], ip[3]);

  return output_len;
#endif  
} /* }}} */

/* parse an ip address at cmd, write result to ptr */
int8_t parse_ip(char *cmd, uip_ipaddr_t *ptr)
/* {{{ */ {

#ifdef DEBUG_ECMD_IP
    debug_printf("called parse_ip with string '%s'\n", cmd);
#endif

#if UIP_CONF_IPV6
    uint16_t *ip = (uint16_t *) ptr;
    int8_t ret = sscanf_P(cmd, PSTR("%x:%x:%x:%x:%x:%x:%x:%x"),
			  ip + 0, ip + 1, ip + 2, ip + 3, 
			  ip + 4, ip + 5, ip + 6, ip + 7);
    
    if (ret != 8)
	return -1;

    for (int i = 0; i < 8; i ++)
	ip[i] = HTONS (ip[i]);

#else
    uint8_t *ip = (uint8_t *) ptr;
    int8_t ret = sscanf_P(cmd, PSTR("%u.%u.%u.%u"), ip, ip+1, ip+2, ip+3);

    if (ret != 4) 
	return -1;
#endif

    return 0;
} /* }}} */

#ifdef ENC28J60_SUPPORT
/* parse an ethernet address at cmd, write result to ptr */
static int8_t parse_mac(char *cmd, uint8_t *ptr)
/* {{{ */ {

#ifdef DEBUG_ECMD_MAC
    debug_printf("called parse_mac with string '%s'\n", cmd);
#endif

    int *mac = __builtin_alloca(sizeof(int) * 6);

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

    return ret;
} /* }}} */

int16_t parse_cmd_mac(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    (void) output;
    (void) len;

#ifdef DEBUG_ECMD_MAC
    debug_printf("called with string %s\n", cmd);
#endif

    int8_t ret;

    /* allocate space for mac */
    struct uip_eth_addr new_mac;

    ret = parse_mac(cmd, (void *)&new_mac);

    if (ret >= 0) {
        eeprom_save(mac, &new_mac, 6);
        eeprom_update_chksum();
        return 0;
    }
    else
        return ret;

} /* }}} */

int16_t parse_cmd_show_mac(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    (void) cmd;

#ifdef DEBUG_ECMD_MAC
    debug_printf("called parse_cmd_show with rest: \"%s\"\n", cmd);
#endif

    struct uip_eth_addr buf;
    uint8_t *saved_mac = (uint8_t *)&buf;

    eeprom_restore(mac, saved_mac, 6);

    int output_len = snprintf_P(output, len,
            PSTR("%02x:%02x:%02x:%02x:%02x:%02x"),
            saved_mac[0], saved_mac[1],
            saved_mac[2], saved_mac[3],
            saved_mac[4], saved_mac[5]);

    return output_len;
} /* }}} */
#endif /* ENC28J60_SUPPORT */

#if (!defined(IPV6_SUPPORT) && !defined(BOOTP_SUPPORT))		\
  || defined(IPV6_STATIC_SUPPORT)
int16_t parse_cmd_ip(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    uip_ipaddr_t hostaddr;

    while (*cmd == ' ')
	cmd++;

    /* try to parse ip */
    if (parse_ip (cmd, &hostaddr))
	return -1;

    eeprom_save(ip, &hostaddr, IPADDR_LEN);
    eeprom_update_chksum();
    
    return 0;
} /* }}} */
#endif /* IPv4-static || IPv6-static || OpenVPN */

#if !UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)
int16_t parse_cmd_netmask(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    uip_ipaddr_t new_netmask;

    while (*cmd == ' ')
	cmd++;

    /* try to parse ip */
    if (parse_ip (cmd, &new_netmask))
	return -1;

    eeprom_save(netmask, &new_netmask, IPADDR_LEN);
    eeprom_update_chksum();
    
    return 0;
} /* }}} */
#endif /* !UIP_CONF_IPV6 and !BOOTP_SUPPORT */

#if (!UIP_CONF_IPV6 || IPV6_STATIC_SUPPORT) && !defined(BOOTP_SUPPORT)
int16_t parse_cmd_gw(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    uip_ipaddr_t gwaddr;

    while (*cmd == ' ')
	cmd++;

    /* try to parse ip */
    if (parse_ip (cmd, &gwaddr))
	return -1;

    eeprom_save(gateway, &gwaddr, IPADDR_LEN);
    eeprom_update_chksum();

    return 0;
} /* }}} */
#endif /* !UIP_CONF_IPV6 and !BOOTP_SUPPORT */

int16_t parse_cmd_show_ip(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    (void) cmd;

    uip_ipaddr_t hostaddr;
    uip_gethostaddr(&hostaddr);

    return print_ipaddr (&hostaddr, output, len);
} /* }}} */

#ifndef IPV6_SUPPORT
int16_t parse_cmd_show_netmask(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    (void) cmd;

    uip_ipaddr_t netmask;
    uip_getnetmask(&netmask);

    return print_ipaddr (&netmask, output, len);
} /* }}} */
#endif /* !IPV6_SUPPORT */

int16_t parse_cmd_show_gw(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    (void) cmd;

    uip_ipaddr_t draddr;
    uip_getdraddr(&draddr);

    return print_ipaddr (&draddr, output, len);
} /* }}} */

#endif /* not TEENSY_SUPPORT */

#endif  /* not UIP_SUPPORT */
