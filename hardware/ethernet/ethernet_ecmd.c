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

int16_t print_mac(struct uip_eth_addr *mac, char *output, uint16_t len)
{
    uint8_t *addr = mac->addr;

    return snprintf_P(output, len, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"),
		      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}


#ifndef DISABLE_IPCONF_SUPPORT
/* parse an ethernet address at cmd, write result to ptr */
int8_t parse_mac(char *cmd, struct uip_eth_addr *mac)
{
    if (mac != NULL) {
	uint8_t *addr = mac->addr;

#ifdef DEBUG_ECMD_MAC
	debug_printf("called parse_mac with string '%s'\n", cmd);
#endif

	int ret = sscanf_P(cmd, PSTR("%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx"),
			   addr, addr+1, addr+2, addr+3, addr+4, addr+5);

#ifdef DEBUG_ECMD_MAC
	debug_printf("scanf returned %d\n", ret);
#endif

	if (ret == 6) {
#ifdef DEBUG_ECMD_MAC
	    debug_printf("read mac %x:%x:%x:%x:%x:%x\n",
			 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
#endif
	    return 0;
	}
    }

    return -1;
}
#endif /* DISABLE_IPCONF_SUPPORT */



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


/*
  -- Ethersex META --
  block(Network configuration)
  ecmd_feature(mac, "mac",[xx:xx:xx:xx:xx:xx],Display/Set the MAC address.)
  ecmd_ifdef(DEBUG_ENC28J60)
    ecmd_feature(enc_dump, "enc dump", , Dump the internal state of the enc to serial)
  ecmd_endif()
    
*/
