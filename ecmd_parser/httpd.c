/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../eeprom.h"
#include "../usart.h"
#include "ecmd.h"


#if defined(HTTPD_AUTH_SUPPORT)
int16_t parse_cmd_http_passwd(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    struct eeprom_config_ext_t new_cfg;
    memset(&new_cfg, 0, sizeof(new_cfg));
    while (*cmd == ' ') cmd ++;
    if (! *cmd ) { /* No argument */
display_password:
      snprintf_P(output, len, PSTR("password: "));
      eeprom_read_block(output + 10, &(((struct eeprom_config_ext_t *)
                      EEPROM_CONFIG_EXT)->httpd_auth_password),
                      sizeof(new_cfg.httpd_auth_password));
      return strlen(output);
    } else {
      strncpy(new_cfg.httpd_auth_password, cmd, 
              sizeof(new_cfg.httpd_auth_password) - 1);
      new_cfg.httpd_auth_password[sizeof(new_cfg.httpd_auth_password) - 1] = 0;
      eeprom_save_config_ext(&new_cfg);
      goto display_password;
    }
} /* }}} */
#endif
