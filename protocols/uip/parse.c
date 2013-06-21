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

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"

int16_t print_ipaddr(uip_ipaddr_t *addr, char *output, uint16_t len)
{
    if (addr == NULL)
        return snprintf(output, len, "NULL");
#if UIP_CONF_IPV6
    return snprintf_P(output, len, PSTR("%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"),
		      HTONS(((u16_t *)(addr))[0]), HTONS(((u16_t *)(addr))[1]),
		      HTONS(((u16_t *)(addr))[2]), HTONS(((u16_t *)(addr))[3]),
		      HTONS(((u16_t *)(addr))[4]), HTONS(((u16_t *)(addr))[5]),
		      HTONS(((u16_t *)(addr))[6]), HTONS(((u16_t *)(addr))[7]));
#else
    uint8_t *ip = (uint8_t *) addr;
    return snprintf_P(output, len, PSTR("%u.%u.%u.%u"),
		      ip[0], ip[1], ip[2], ip[3]);
#endif  
}


#if !defined(DISABLE_IPCONF_SUPPORT) || defined(NTP_SUPPORT) || defined(DNS_SUPPORT)
/* parse an ip address at cmd, write result to ptr */
int8_t parse_ip(char *cmd, uip_ipaddr_t *ptr)
{
    if (ptr != NULL) {
#ifdef DEBUG_ECMD_IP
	debug_printf("called parse_ip with string '%s'\n", cmd);
#endif

#if UIP_CONF_IPV6
	uint16_t *ip = (uint16_t *) ptr;
	int8_t ret = sscanf_P(cmd,
		PSTR("%4hhx:%4hhx:%4hhx:%4hhx:%4hhx:%4hhx:%4hhx:%4hhx"),
			      ip, ip+1, ip+2, ip+3, ip+4, ip+5, ip+6, ip+7);

#ifdef DEBUG_ECMD_IP
	debug_printf("scanf returned %d\n", ret);
#endif

	if (ret != 8)
	    return -1;

	for (int i = 0; i < 8; i ++)
	    ip[i] = HTONS(ip[i]);
#else
	uint8_t *ip = (uint8_t *) ptr;
	int8_t ret = sscanf_P(cmd, PSTR("%hhu.%hhu.%hhu.%hhu"),
			      ip, ip+1, ip+2, ip+3);

#ifdef DEBUG_ECMD_IP
	debug_printf("scanf returned %d\n", ret);
#endif

	if (ret != 4)
	    return -1;
#endif

	return 0;
    }

    return -1;
}
#endif /* !DISABLE_IPCONF_SUPPORT || NTP_SUPPORT || DNS_SUPPORT */


