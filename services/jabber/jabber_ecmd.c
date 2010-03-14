/*
 * Copyright (c) 2009 by Stefan Müller <mueller@cos-gbr.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "jabber.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"


#ifdef JABBER_EEPROM_SUPPORT

int16_t parse_cmd_user (char *cmd, char *output, uint16_t len)
{
	JABDEBUG ("username\n");
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(jabber_user, 0, JABBER_VALUESIZE);
		memcpy(jabber_user, cmd, JABBER_VALUESIZE);
		eeprom_save(jabber_username, cmd, strlen(cmd));
		eeprom_update_chksum();
		JABDEBUG ("set new : %s\n",cmd);
		return ECMD_FINAL_OK;
    }
    else {
    	JABDEBUG ("get current : %s\n ",jabber_user);
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_user));
    }	
}

int16_t parse_cmd_pass (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(jabber_pass, 0, JABBER_VALUESIZE);
		memcpy(jabber_pass, cmd, JABBER_VALUESIZE);
		eeprom_save(jabber_password, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_pass));
    }	
}

int16_t parse_cmd_resrc (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(jabber_resrc, 0, JABBER_VALUESIZE);
		memcpy(jabber_resrc, cmd, JABBER_VALUESIZE);
		eeprom_save(jabber_resource, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_resrc));
    }	
}

int16_t parse_cmd_host (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(jabber_host, 0, JABBER_VALUESIZE);
		memcpy(jabber_host, cmd, JABBER_VALUESIZE);
		eeprom_save(jabber_hostname, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_host));
    }	
}

#endif


/*
  -- Ethersex META --
  block([[JABBER]])
ecmd_ifdef(JABBER_EEPROM_SUPPORT)
  ecmd_feature(user, "jabber_user", [USERNAME], JABBER username)
  ecmd_feature(pass, "jabber_pass", [PASSWORD], JABBER password)
  ecmd_feature(pass, "jabber_resrc", [RESOURCE], JABBER resource)
  ecmd_feature(pass, "jabber_host", [HOSTNAME], JABBER hostname)
ecmd_endif()
*/

