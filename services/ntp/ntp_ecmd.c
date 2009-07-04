/*
 *
 * Copyright (c) 2009 Peter Marschall <peter@adpm.de>
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

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "protocols/dns/resolv.h"
#include "ntp.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_ntp_server(char *cmd, char *output, uint16_t len)
{
    uip_ipaddr_t ntpaddr;

    while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
	/* try to parse ip */
	if (parse_ip(cmd, &ntpaddr) != 0) {
#ifdef DNS_SUPPORT
	    uip_ipaddr_t *ip;

	    if (!(ip = resolv_lookup(cmd)))
		resolv_query(cmd, ntp_dns_query_cb);
	    else
		ntp_conf(ip);
#else
	    return ECMD_ERR_PARSE_ERROR;
#endif
	}
	else
	    ntp_conf(&ntpaddr);

	return ECMD_FINAL_OK;
    }
    else {
	return ECMD_FINAL(print_ipaddr(ntp_getserver(), output, len));
    }
}

int16_t parse_cmd_ntp_query(char *cmd, char *output, uint16_t len)
{
    ntp_send_packet();

    return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  ecmd_feature(ntp_query, "ntp query",, Query the NTP server to get an NTP update.)
  ecmd_feature(ntp_server, "ntp server", [IPADDR], Display/Set the IP address of the NTP server to use to IPADDR.)
*/
