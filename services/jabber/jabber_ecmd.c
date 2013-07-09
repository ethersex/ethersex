/*
 * Copyright (c) 2009 Stefan Müller <mueller@cos-gbr.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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
#include "jabber_ecmd.h"


int16_t
parse_cmd_user(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd != '\0')
  {
    strncpy(jabber_user, cmd, sizeof(jabber_user));
    jabber_user[sizeof(jabber_user) - 1] = '\0';
    eeprom_save(jabber_username, jabber_user, JABBER_VALUESIZE);
    eeprom_update_chksum();
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_user));
}

int16_t
parse_cmd_pass(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd != '\0')
  {
    strncpy(jabber_pass, cmd, sizeof(jabber_pass));
    jabber_pass[sizeof(jabber_pass) - 1] = '\0';
    eeprom_save(jabber_password, jabber_pass, JABBER_VALUESIZE);
    eeprom_update_chksum();
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_pass));
}

int16_t
parse_cmd_resrc(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd != '\0')
  {
    strncpy(jabber_resrc, cmd, sizeof(jabber_resrc));
    jabber_resrc[sizeof(jabber_resrc) - 1] = '\0';
    eeprom_save(jabber_resource, jabber_resrc, JABBER_VALUESIZE);
    eeprom_update_chksum();
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_resrc));
}

int16_t
parse_cmd_host(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd != '\0')
  {
    strncpy(jabber_host, cmd, sizeof(jabber_host));
    jabber_host[sizeof(jabber_host) - 1] = '\0';
    eeprom_save(jabber_hostname, jabber_host, JABBER_VALUESIZE);
    eeprom_update_chksum();
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), jabber_host));
}


/*
  -- Ethersex META --

  block([[Jabber]])
  ecmd_feature(user,  "jabber_user",  [USERNAME], JABBER username)
  ecmd_feature(pass,  "jabber_pass",  [PASSWORD], JABBER password)
  ecmd_feature(resrc, "jabber_resrc", [RESOURCE], JABBER resource)
  ecmd_feature(host,  "jabber_host",  [HOSTNAME], JABBER hostname)
*/
