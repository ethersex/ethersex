/*
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
    uip_ipaddr_t gwaddr;

    while (*cmd == ' ')
	cmd++;

#ifndef DISABLE_IPCONF_SUPPORT
#if (!UIP_CONF_IPV6 || IPV6_STATIC_SUPPORT) && !defined(BOOTP_SUPPORT)

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
        uip_getdraddr(&gwaddr);

        return ECMD_FINAL(print_ipaddr(&gwaddr, output, len));
    }
}

/*
  -- Ethersex META --
  block(Network configuration)
  ecmd_ifndef(TEENSY_SUPPORT)
    ecmd_ifdef(UIP_SUPPORT)
      ecmd_ifndef(IPV6_SUPPORT)
	ecmd_feature(netmask, "netmask",[IP],Display/Set the network mask.)
      ecmd_endif()

      ecmd_feature(ip, "ip",[IP],Display/Set the IP address.)
      ecmd_feature(gw, "gw",[IP],Display/Set the address of the default router.)
    ecmd_endif()
  ecmd_endif()
*/
