/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Jochen Roessner <jochen@lugrot.de>
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
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "ecmd.h"


#ifdef CONTROL6_SUPPORT
int16_t parse_cmd_c6_get(char *cmd, char *output, uint16_t len)
/* {{{ */ {
  char *varname;
  uint8_t varvalue;

  uint8_t ret = sscanf_P(cmd, PSTR("%s %u"), &varname, &varvalue);
  if (ret == 2) {
    
    return snprintf_P(output, len, "%s %u");
  } else
    return -1;
}
/* }}} */

int16_t parse_cmd_c6_set(char *cmd, char *output, uint16_t len)
/* {{{ */ {
  char *varname;
  uint8_t varvalue;
  
  uint8_t ret = sscanf_P(cmd, PSTR("%s %u"), &varname, &varvalue);
  
  if ( ret == 2 ) {
  
  }

    return -1;
}
/* }}} */

/* }}} */
#endif /* CONTROL6_SUPPORT */

