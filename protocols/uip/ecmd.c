/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
 */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"


#ifndef TEENSY_SUPPORT

#ifdef ENC28J60_SUPPORT
int16_t parse_cmd_mac(char *cmd, char *output, uint16_t len)
{
    (void) output;
    (void) len;

#ifdef DEBUG_ECMD_MAC
    debug_printf("parse_cmd_mac() called with string %s\n", cmd);
#endif

#ifndef DISABLE_IPCONF_SUPPORT
    while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
	int8_t ret;

	/* allocate space for mac */
	struct uip_eth_addr new_mac;

	ret = parse_mac(cmd, (void *) &new_mac);

	if (ret >= 0) {
	    eeprom_save(mac, &new_mac, 6);
	    eeprom_update_chksum();
	    return ECMD_FINAL_OK;
	}
	else
	    return ECMD_ERR_PARSE_ERROR;
    }
    else
#endif /* DISABLE_IPCONF_SUPPORT */
    {
	struct uip_eth_addr buf;
	uint8_t *saved_mac = (uint8_t *) &buf;

	eeprom_restore(mac, saved_mac, 6);

	return ECMD_FINAL(print_mac(&buf, output, len));
    }
}
#endif /* ENC28J60_SUPPORT */

int16_t parse_cmd_ip(char *cmd, char *output, uint16_t len)
{
    uip_ipaddr_t hostaddr;

    while (*cmd == ' ')
	cmd++;

#ifndef DISABLE_IPCONF_SUPPORT
#if (!defined(IPV6_SUPPORT) && !defined(BOOTP_SUPPORT))		\
  || defined(IPV6_STATIC_SUPPORT)
    if (*cmd != '\0') {
        /* try to parse ip */
        if (parse_ip(cmd, &hostaddr))
	    return ECMD_ERR_PARSE_ERROR;

        eeprom_save(ip, &hostaddr, IPADDR_LEN);
        eeprom_update_chksum();

        return ECMD_FINAL_OK;
    }
    else
#endif /* IPv4-static || IPv6-static || OpenVPN */
#endif /* DISABLE_IPCONF_SUPPORT */
    {
        uip_gethostaddr(&hostaddr);

        return ECMD_FINAL(print_ipaddr(&hostaddr, output, len));
    }
}

#ifndef IPV6_SUPPORT
int16_t parse_cmd_netmask(char *cmd, char *output, uint16_t len)
{
    uip_ipaddr_t netmask;

    while (*cmd == ' ')
	cmd++;

#ifndef DISABLE_IPCONF_SUPPORT
#if !UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)
    if (*cmd != '\0') {
        /* try to parse ip */
        if (parse_ip (cmd, &netmask))
	    return ECMD_ERR_PARSE_ERROR;

        eeprom_save(netmask, &netmask, IPADDR_LEN);
        eeprom_update_chksum();

        return ECMD_FINAL_OK;
    }
    else
#endif /* !UIP_CONF_IPV6 and !BOOTP_SUPPORT */
#endif /* DISABLE_IPCONF_SUPPORT */
    {
        uip_getnetmask(&netmask);

        return ECMD_FINAL(print_ipaddr(&netmask, output, len));
    }
}
#endif /* !IPV6_SUPPORT */

int16_t parse_cmd_gw(char *cmd, char *output, uint16_t len)
{
    while (*cmd == ' ')
	cmd++;

#ifndef DISABLE_IPCONF_SUPPORT
#if (!UIP_CONF_IPV6 || IPV6_STATIC_SUPPORT) && !defined(BOOTP_SUPPORT)
    uip_ipaddr_t gwaddr;

    if (*cmd != '\0') {
        /* try to parse ip */
        if (parse_ip (cmd, &gwaddr))
	    return ECMD_ERR_PARSE_ERROR;

        eeprom_save(gateway, &gwaddr, IPADDR_LEN);
        eeprom_update_chksum();

        return ECMD_FINAL_OK;
    }
    else
#endif /* !UIP_CONF_IPV6 and !BOOTP_SUPPORT */
#endif /* DISABLE_IPCONF_SUPPORT */
    {
    uip_ipaddr_t gwaddr;
        uip_getdraddr(&gwaddr);

        return ECMD_FINAL(print_ipaddr(&gwaddr, output, len));
    }
}

#endif /* not TEENSY_SUPPORT */

#ifdef IPSTATS_SUPPORT
int16_t parse_cmd_ipstats(char *cmd, char *output, uint16_t len)
{
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != 23) {	/* indicator flag: real invocation:  0 */
    cmd[0] = 23;	/*                 continuing call: 23 */
    cmd[1] = 0;		/* counter for network interface */
    cmd[2] = 0;		/* counter for output lines for an interface */
  }
  uint8_t tmp;
#ifdef MULTISTACK_SUPPORT
  uip_stack_set_active(cmd[1]);
#endif 

  enum {
    INTERFACE,
    ADDRESS,
    GATEWAY,
    IP,
#ifdef UDP_SUPPORT
    UDP,
#endif
#ifdef TCP_SUPPORT
    TCP,
#endif
#ifdef ICMP_SUPPORT
    ICMP,
#endif
    LAST
  };

  switch (cmd[2]) {
  case INTERFACE:
    len = snprintf_P(output, len, PSTR("iface %d:"), cmd[1]);
    break;
  case ADDRESS:
    output[0] = ' ';
    output[1] = ' ';
    tmp = len;
    len = 2 + print_ipaddr (&uip_hostaddr, output + 2, len - 2);
    output[len++] = '/';
#ifdef IPV6_SUPPORT
    len += snprintf_P(output + len, tmp - len, PSTR("%d"), uip_prefix_len);
#else
    len += print_ipaddr(&uip_netmask, output + len, tmp - len);
#endif
    break;
  case GATEWAY:
    snprintf_P(output, len, PSTR("  gw: "));
    len = 6 + print_ipaddr (&uip_draddr, output + 6, len - 6);
    break;
  case IP:
    len = snprintf_P(output, len, PSTR("  ip:   recv %5d sent %5d drop %5d"), 
                     uip_stat.ip.recv, uip_stat.ip.sent, uip_stat.ip.drop);
    break;
#ifdef ICMP_SUPPORT
  case ICMP:
    len = snprintf_P(output, len, PSTR("  icmp: recv %5d sent %5d drop %5d"), 
                     uip_stat.icmp.recv, uip_stat.icmp.sent, uip_stat.icmp.drop);
    break;
#endif
#ifdef UDP_SUPPORT
  case UDP:
    len = snprintf_P(output, len, PSTR("  udp:  recv %5d sent %5d drop %5d"), 
                     uip_stat.udp.recv, uip_stat.udp.sent, uip_stat.udp.drop);
    break;
#endif
#ifdef TCP_SUPPORT
  case TCP:
    len = snprintf_P(output, len, PSTR("  tcp:  recv %5d sent %5d drop %5d"), 
                     uip_stat.tcp.recv, uip_stat.tcp.sent, uip_stat.tcp.drop);
    break;
#endif
  }
  cmd[2]++;
  if (cmd[2] == LAST) {
    cmd[2] = 0;
    cmd[1] ++;
    if (cmd[1] == STACK_LEN)
      return ECMD_FINAL(len);
  }

  return ECMD_AGAIN(len);
}
#endif /* IPSTATS_SUPPORT */

