/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
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
#include "../portio.h"
#include "../kty/kty81.h"
#include "ecmd.h"

#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'A')) 

#ifdef KTY_SUPPORT



int16_t parse_cmd_kty_get(char *cmd, char *output, uint16_t len)
/* {{{ */ {
  uint16_t adc;
  int16_t temp;
  uint8_t channel;
  uint8_t ret = 0;
  if (cmd[0] && cmd[1]) {
    if ( (cmd[1] - '0') < ADC_CHANNELS) {
      adc = get_kty(cmd[1] - '0');
      channel = ADC_CHANNELS;
      goto adc_out; 
    } else 
      return -1;
  }
  for (channel = 0; channel < ADC_CHANNELS; channel ++) {
    adc = get_kty(channel);
adc_out:
    temp = temperatur(adc);
    temp2text(output, temp);
    ret += 6;
    output += 6;
  }
  return ret;
} /* }}} */
#endif
