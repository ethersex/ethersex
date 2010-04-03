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
#include "core/eeprom.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"
#include "services/pam/pam_prototypes.h"


void 
pam_auth(char *username, char *password, uint8_t *auth_state) 
{
  char new_user[sizeof(((struct eeprom_config_t * )0x0000)->pam_username) + 1];
  char new_pass[sizeof(((struct eeprom_config_t * )0x0000)->pam_password) + 1];

  memset(new_user, 0, sizeof(new_user));
  memset(new_pass, 0, sizeof(new_pass));

  eeprom_restore(pam_username, new_user, sizeof(new_user));
  eeprom_restore(pam_password, new_pass, sizeof(new_pass));

  if (strcmp(new_user, username) != 0 
      || strcmp(new_pass, password) != 0)
    *auth_state = PAM_DENIED;
  else
    *auth_state = PAM_SUCCESS;
}

int16_t parse_cmd_pam(char *cmd, char *output, uint16_t len)
{
    char new_user[sizeof(((struct eeprom_config_t * )0x0000)->pam_username) + 1];
    char new_pass[sizeof(((struct eeprom_config_t * )0x0000)->pam_password) + 1];

    memset(new_user, 0, sizeof(new_user));
    memset(new_pass, 0, sizeof(new_pass));

    while (*cmd == ' ') cmd ++;
    if (! *cmd ) { /* No argument */
display_password:
      eeprom_restore(pam_username, new_user, sizeof(new_user));
      eeprom_restore(pam_password, new_pass, sizeof(new_pass));
      return ECMD_FINAL(snprintf_P(output, len, PSTR("user: '%s' passwd: '%s'"),
                                   new_user, new_pass));
    } else {
      char *password = strchr(cmd, ' ');
      if (!password) return ECMD_ERR_PARSE_ERROR;
      *password = 0;
      password ++;
      while (*password == ' ') password ++;

      char *p = strchr(password, ' ');
      if (p) *p = 0;

      strncpy(new_user, cmd, sizeof(new_user) - 2);
      strncpy(new_pass, password, sizeof(new_pass) - 2);

      new_user[sizeof(new_user) - 2] = 0;
      new_pass[sizeof(new_pass) - 2] = 0;

      eeprom_save(pam_username, new_user, sizeof(new_user));
      eeprom_save(pam_password, new_pass, sizeof(new_pass));
      eeprom_update_chksum();

      goto display_password;
    }
}

/*
  -- Ethersex META --
  block(Miscelleanous)
  ecmd_feature(pam, "pam", USER PASSWORD, Use pam for user and password)
*/
