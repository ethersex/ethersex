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
#include "sms77.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_sm (char *cmd, char *output, uint16_t len) 
{
  if (sms77_send(cmd)) 
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(cmd, len, PSTR("sending failed")));
}

#ifdef SMS77_EEPROM_SUPPORT

int16_t parse_cmd_user (char *cmd, char *output, uint16_t len)
{
	SMSDEBUG ("username\n");
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(sms77_user, 0, SMS77_VALUESIZE);
		memcpy(sms77_user, cmd, SMS77_VALUESIZE);
		eeprom_save(sms77_username, cmd, strlen(cmd));
		eeprom_update_chksum();
		SMSDEBUG ("set new : %s\n",cmd);
		return ECMD_FINAL_OK;
    }
    else {
    	SMSDEBUG ("get current : %s\n ",sms77_user);
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), sms77_user));
    }	
}

int16_t parse_cmd_pass (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		memset(sms77_pass, 0, SMS77_VALUESIZE);
		memcpy(sms77_pass, cmd, SMS77_VALUESIZE);
		eeprom_save(sms77_password, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), sms77_pass));
    }	
}

int16_t parse_cmd_recv (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
	
		memset(sms77_recv, 0, SMS77_VALUESIZE);
		memcpy(sms77_recv, cmd, SMS77_VALUESIZE);
		eeprom_save(sms77_receiver, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), sms77_recv));
    }	
}

int16_t parse_cmd_type (char *cmd, char *output, uint16_t len)
{
	
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
	
		memset(sms77_type, 0, SMS77_VALUESIZE);
		memcpy(sms77_type, cmd, SMS77_VALUESIZE);
		eeprom_save(sms77_type, cmd, strlen(cmd));
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    }
    else {
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), sms77_type));
    }	
}

#endif


/*
  -- Ethersex META --
  block([[SMS77]])
  ecmd_feature(sm, "sms77 ",MESSAGE,Send MESSAGE to compiled in sms77 service)
ecmd_ifdef(SMS77_EEPROM_SUPPORT)
  ecmd_feature(user, "sms77_user", [USERNAME], SMS77 username)
  ecmd_feature(pass, "sms77_pass", [PASSWORD], SMS77 password)
  ecmd_feature(recv, "sms77_recv", [RECEIVER], SMS receiver)
  ecmd_feature(type, "sms77_type", [TYPE], SMS type)
ecmd_endif()
*/

