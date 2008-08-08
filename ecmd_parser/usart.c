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


#if defined(USART_SUPPORT) && !defined(TEENSY_SUPPORT)
int16_t parse_cmd_usart_baud(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    while (*cmd == ' ') cmd ++;
    if (! *cmd ) { /* No argument */
      return snprintf_P(output, len, PSTR("baudrate: %d00"),
                 eeprom_read_word(&(((struct eeprom_config_ext_t *)
                                     EEPROM_CONFIG_EXT)->usart_baudrate)));
    } else {
      /* Delete the last two digits */
      cmd[strlen(cmd) - 2] = 0;
      struct eeprom_config_ext_t new_cfg;
      memset(&new_cfg, 0, sizeof(new_cfg));
      if (sscanf_P(cmd, PSTR("%d"), &new_cfg.usart_baudrate) == 1) {
        usart_baudrate(new_cfg.usart_baudrate);
        eeprom_save_config_ext(&new_cfg);
        return snprintf_P(output, len, PSTR("baudrate: %d00"), new_cfg.usart_baudrate);
      } else 
        return -1;
    }
} /* }}} */
#endif
