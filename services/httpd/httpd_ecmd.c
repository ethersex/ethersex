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
#include <avr/interrupt.h>

#include "config.h"
#include "debug.h"
#include "uip/uip.h"
#include "eeprom.h"
#include "usart.h"
#include "ecmd_parser/ecmd.h"


#if defined(HTTPD_AUTH_SUPPORT)
int16_t parse_cmd_http_passwd(char *cmd, char *output, uint16_t len)
{
    char new_pass[sizeof(((struct eeprom_config_t * )0x0000)->httpd_auth_password) + 1];

    while (*cmd == ' ') cmd ++;
    if (! *cmd ) { /* No argument */
display_password:
      snprintf_P(output, len, PSTR("password: "));
      eeprom_restore(httpd_auth_password, output + 10, sizeof(new_pass));
      return strlen(output);
    } else {
      strncpy(new_pass, cmd, sizeof(new_pass) - 1);
      /* The last byte MUST be a null byte. It will be
       * char httpd_auth_null_byte in the eeprom struct
       */
      new_pass[sizeof(new_pass) - 1] = 0;

      eeprom_save(httpd_auth_password, new_pass, sizeof(new_pass));
      eeprom_update_chksum();

      goto display_password;
    }
}
#endif
