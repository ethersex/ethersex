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
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "core/portio/portio.h"

#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'A')) 

#ifndef ADC_REF
#define ADC_REF 0
#endif

int16_t parse_cmd_adc_get(char *cmd, char *output, uint16_t len)
{
  uint16_t adc;
  uint8_t channel;
  uint8_t ret = 0;
  if (cmd[0] && cmd[1]) {
    if ( (cmd[1] - '0') < ADC_CHANNELS) {
      ADMUX = (ADMUX & 0xF0) | (cmd[1] - '0') | ADC_REF;
      channel = ADC_CHANNELS;
      goto adc_out; 
    } else 
      return -1;
  }
  for (channel = 0; channel < ADC_CHANNELS; channel ++) {
    ADMUX = (ADMUX & 0xF0) | channel | ADC_REF;
adc_out:
    /* Start adc conversion */
    ADCSRA |= _BV(ADSC);
    /* Wait for completion of adc */
    while (ADCSRA & _BV(ADSC)) {}
    adc = ADC;
    output[0] = NIBBLE_TO_HEX((adc >> 8) & 0x0F);
    output[1] = NIBBLE_TO_HEX((adc >> 4) & 0x0F);
    output[2] = NIBBLE_TO_HEX(adc & 0x0F);
    output[3] = ' ';
    output[4] = 0;
    ret += 4;
    output += 4;
  }
  return ret;
}
