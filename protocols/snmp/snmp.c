/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "services/clock/clock.h"
#include "snmp.h"


#ifdef SNMP_SUPPORT

/**********************************************************
 * helper functions 
 **********************************************************/

uint8_t
encode_int(uint8_t * ptr, uint16_t val)
{
  ptr[0] = SNMP_TYPE_INTEGER;
  ptr[1] = 2;
  ptr[2] = HI8(val);
  ptr[3] = LO8(val);
  return 4;
}

uint8_t
encode_long(uint8_t * ptr, uint32_t val)
{
  ptr[0] = SNMP_TYPE_INTEGER;
  ptr[1] = 4;
  *((uint32_t *) (ptr + 2)) = HTONL(val);
  return 6;
}

uint8_t
onelevel_next(uint8_t * ptr, struct snmp_varbinding * bind, uint8_t count)
{
  if (bind->len == 0)
  {
    ptr[0] = 0;
    return 1;
  }
  if (bind->len == 1 && bind->data[0] < (count - 1))
  {
    ptr[0] = bind->data[0] + 1;
    return 1;
  }
  return 0;
}

/**********************************************************
 * reactions
 **********************************************************/

#ifdef WHM_SUPPORT
uint8_t
uptime_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 0)
  {
    return 0;
  }
  return encode_long(ptr, clock_get_uptime() * 100L);
}
#endif

#ifdef ADC_SUPPORT
uint8_t
adc_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= ADC_CHANNELS)
  {
    return 0;
  }
  ADMUX = (ADMUX & 0xF0) | bind->data[0];
  /* Start adc conversion */
  ADCSRA |= _BV(ADSC);
  /* Wait for completion of adc */
  while (ADCSRA & _BV(ADSC))
  {
  }
  return encode_int(ptr, ADC);
}

uint8_t
adc_next(uint8_t * ptr, struct snmp_varbinding * bind)
{
  return onelevel_next(ptr, bind, ADC_CHANNELS);
}
#endif

uint8_t
string_pgm_reaction(uint8_t * ptr, struct snmp_varbinding * bind,
                    void *userdata)
{
  if (bind->len != 0)
  {
    return 0;
  }
  ptr[0] = SNMP_TYPE_STRING;
  ptr[1] = strlen_P((char *) userdata);
  memcpy_P(ptr + 2, userdata, ptr[1]);
  return ptr[1] + 2;
}

const char desc_value[] PROGMEM = SNMP_VALUE_DESCRIPTION;
const char desc_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x01";

#ifdef WHM_SUPPORT
const char uptime_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x03";
#endif

const char contact_value[] PROGMEM = SNMP_VALUE_CONTACT;
const char contact_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x04";

const char hostname_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x05";
const char hostname_value[] PROGMEM = CONF_HOSTNAME;

const char location_value[] PROGMEM = SNMP_VALUE_LOCATION;
const char location_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x06";

#ifdef ADC_SUPPORT
const char adc_reaction_obj_name[] PROGMEM = ethersexExperimental "\x01";
#endif

const struct snmp_reaction snmp_reactions[] PROGMEM = {
  {desc_obj_name, string_pgm_reaction, (void *) desc_value, NULL},
#ifdef WHM_SUPPORT
  {uptime_reaction_obj_name, uptime_reaction, NULL, NULL},
#endif
  {contact_obj_name, string_pgm_reaction, (void *) contact_value, NULL},
  {hostname_reaction_obj_name, string_pgm_reaction, (void *) hostname_value, NULL},
  {location_obj_name, string_pgm_reaction, (void *) location_value, NULL},
#ifdef ADC_SUPPORT
  {adc_reaction_obj_name, adc_reaction, NULL, adc_next},
#endif
  {NULL, NULL, NULL, NULL}
};

#endif
