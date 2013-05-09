/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#include "resolv.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_dns_server(char *cmd, char *output, uint16_t len)
{
    uip_ipaddr_t dnsaddr;

    while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
	/* try to parse ip */
	if (parse_ip(cmd, &dnsaddr))
	    return ECMD_ERR_PARSE_ERROR;

	resolv_conf(&dnsaddr);

	eeprom_save(dns_server, &dnsaddr, IPADDR_LEN);
	eeprom_update_chksum();
	return ECMD_FINAL_OK;
    }
    else {
	return ECMD_FINAL(print_ipaddr(resolv_getserver(), output, len));
    }
}

int16_t parse_cmd_nslookup (char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ') cmd ++;
  uip_ipaddr_t *addr = resolv_lookup (cmd);

  if (addr) {
    return ECMD_FINAL(print_ipaddr(addr, output, len));
  }
  else {
    resolv_query (cmd, NULL);
    return ECMD_FINAL(snprintf_P(output, len, PSTR("nslookup triggered, try again for result.")));
  }
}

/*
  -- Ethersex META --
  block(DNS Resolver)
  ecmd_feature(nslookup, "nslookup ", HOSTNAME, Do DNS lookup for HOSTNAME (call twice).)
  ecmd_feature(dns_server, "dns server", [IPADDR], Display/Set the IP address of the DNS server to use to IPADDR.)
*/
