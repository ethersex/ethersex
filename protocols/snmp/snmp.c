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
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "services/clock/clock.h"
#include "snmp.h"

#ifdef ADC_SUPPORT
#include "hardware/adc/adc.h"
#endif

#ifdef ONEWIRE_SNMP_SUPPORT
#include "hardware/onewire/onewire.h"
#endif

#ifdef DHT_SNMP_SUPPORT
#include "hardware/dht/dht.h"
#endif

#ifdef TANKLEVEL_SUPPORT
#include "services/tanklevel/tanklevel.h"
#endif

#ifdef SNMP_SUPPORT

/**********************************************************
 * helper functions 
 **********************************************************/

#define TIMESTAMP_TEXT_FORMAT "%02d.%02d.%04d %02d:%02d:%02d"
#define TIMESTAMP_TEXT_LENGTH 20

uint8_t
encode_short(uint8_t * ptr, uint8_t type, uint16_t val)
{
  ptr[0] = type;
  ptr[1] = 2;
  ptr[2] = HI8(val);
  ptr[3] = LO8(val);
  return 4;
}

uint8_t
encode_long(uint8_t * ptr, uint8_t type, uint32_t val)
{
  ptr[0] = type;
  ptr[1] = 4;
  *((uint32_t *) (ptr + 2)) = HTONL(val);
  return 6;
}

uint8_t
encode_timeticks(uint8_t * ptr, timestamp_t ts)
{
  return encode_long(ptr, SNMP_TYPE_TIMETICKS, ts * 100L);
}

uint8_t
encode_timestamp_text(uint8_t * ptr, timestamp_t ts)
{
  clock_datetime_t dt;

  memset(&dt, 0, sizeof(dt));
  clock_localtime(&dt, ts);
  ptr[0] = SNMP_TYPE_STRING;
  ptr[1] = snprintf_P((char *) (ptr + 2),
                      TIMESTAMP_TEXT_LENGTH,
                      PSTR(TIMESTAMP_TEXT_FORMAT),
                      dt.day, dt.month, dt.year + 1900, dt.hour, dt.min,
                      dt.sec);
  return ptr[1] + 2;
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

#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT)
uint8_t
uptime_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 0)
  {
    return 0;
  }
  return encode_timeticks(ptr, clock_get_uptime());
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
  return encode_short(ptr, SNMP_TYPE_INTEGER, adc_get(bind->data[0]));
}

#ifdef ADC_VOLTAGE_SUPPORT

uint8_t
adc_volt_reaction(uint8_t * ptr, struct snmp_varbinding * bind,
                  void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= ADC_CHANNELS)
  {
    return 0;
  }
  return encode_short(ptr, SNMP_TYPE_INTEGER, adc_get_voltage(bind->data[0]));
}

uint8_t
adc_vref_reaction(uint8_t * ptr, struct snmp_varbinding * bind,
                  void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= ADC_CHANNELS)
  {
    return 0;
  }
  return encode_short(ptr, SNMP_TYPE_INTEGER, adc_get_vref());
}

#endif

uint8_t
adc_next(uint8_t * ptr, struct snmp_varbinding * bind)
{
  return onelevel_next(ptr, bind, ADC_CHANNELS);
}
#endif

#ifdef ONEWIRE_SNMP_SUPPORT
uint8_t
ow_rom_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= OW_SENSORS_COUNT)
  {
    return 0;
  }
  uint8_t i = bind->data[0];

  ptr[0] = SNMP_TYPE_STRING;
  ptr[1] = (uint8_t) snprintf_P((char *) (ptr + 2), 17,
                                PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"),
                                ow_sensors[i].ow_rom_code.bytewise[0],
                                ow_sensors[i].ow_rom_code.bytewise[1],
                                ow_sensors[i].ow_rom_code.bytewise[2],
                                ow_sensors[i].ow_rom_code.bytewise[3],
                                ow_sensors[i].ow_rom_code.bytewise[4],
                                ow_sensors[i].ow_rom_code.bytewise[5],
                                ow_sensors[i].ow_rom_code.bytewise[6],
                                ow_sensors[i].ow_rom_code.bytewise[7]);
  return ptr[1] + 2;
}

#ifdef ONEWIRE_NAMING_SUPPORT
uint8_t
ow_name_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= OW_SENSORS_COUNT)
  {
    return 0;
  }
  uint8_t i = bind->data[0];

  ptr[0] = SNMP_TYPE_STRING;
  ptr[1] = 0;
  if (ow_sensors[i].named)
  {
    ptr[1] = (uint8_t) snprintf_P((char *) (ptr + 2), OW_NAME_LENGTH,
                                  PSTR("%s"), ow_sensors[i].name);
  }
  return ptr[1] + 2;
}
#endif

uint8_t
ow_temp_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= OW_SENSORS_COUNT)
  {
    return 0;
  }
  uint8_t i = bind->data[0];

  // always return as twodigits - even if not accurate enough for second
  // digit. Simply multiply temp by 10.
  return encode_short(ptr, SNMP_TYPE_INTEGER, ow_sensors[i].temp.twodigits ?
		      ow_sensors[i].temp.val : ow_sensors[i].temp.val * 10);
}

uint8_t
ow_present_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= OW_SENSORS_COUNT)
  {
    return 0;
  }
  uint8_t i = bind->data[0];

  return encode_short(ptr, SNMP_TYPE_INTEGER, ow_sensors[i].present);
}

uint8_t
ow_next(uint8_t * ptr, struct snmp_varbinding * bind)
{
  return onelevel_next(ptr, bind, OW_SENSORS_COUNT);
}
#endif

#ifdef TANKLEVEL_SUPPORT
uint8_t
tank_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1)
  {
    return 0;
  }

  switch (bind->data[0])
  {
    case 0:
      return encode_short(ptr, SNMP_TYPE_INTEGER, tanklevel_get());
    case 1:
      return encode_short(ptr, SNMP_TYPE_INTEGER, tanklevel_params_ram.ltr_full);
    case 2:
      return encode_long(ptr, SNMP_TYPE_COUNTER, tanklevel_get_ts());
    case 3:
      return encode_timestamp_text(ptr, tanklevel_get_ts());
    default:
      return 0;
  }
}

uint8_t
tank_next(uint8_t * ptr, struct snmp_varbinding * bind)
{
  return onelevel_next(ptr, bind, 4);
}
#endif

#ifdef DHT_SNMP_SUPPORT
uint8_t
dht_polling_delay_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= dht_sensors_count) {
    return 0;
  }
  uint8_t i = bind->data[0];

  return encode_short(ptr, SNMP_TYPE_INTEGER, dht_sensors[i].polling_delay);
}

uint8_t
dht_temp_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= dht_sensors_count) {
    return 0;
  }
  uint8_t i = bind->data[0];

  return encode_short(ptr, SNMP_TYPE_INTEGER, dht_sensors[i].temp);
}

uint8_t
dht_humid_reaction(uint8_t * ptr, struct snmp_varbinding * bind, void *userdata)
{
  if (bind->len != 1 || bind->data[0] >= dht_sensors_count) {
    return 0;
  }
  uint8_t i = bind->data[0];

  return encode_short(ptr, SNMP_TYPE_INTEGER, dht_sensors[i].humid);
}

uint8_t
dht_next(uint8_t * ptr, struct snmp_varbinding * bind)
{
  return onelevel_next(ptr, bind, dht_sensors_count);
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

/**********************************************************
 * mib data
 **********************************************************/

const char desc_value[] PROGMEM = SNMP_VALUE_DESCRIPTION;
const char desc_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x01";

#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT)
const char uptime_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x03";
#endif

const char contact_value[] PROGMEM = SNMP_VALUE_CONTACT;
const char contact_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x04";

const char hostname_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x05";
const char hostname_value[] PROGMEM = CONF_HOSTNAME;

const char location_value[] PROGMEM = SNMP_VALUE_LOCATION;
const char location_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x06";

#ifdef ADC_SUPPORT
const char adc_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x01";
#ifdef ADC_VOLTAGE_SUPPORT
const char adc_volt_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x02\x02";
const char adc_vref_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x02\x03";
#endif
#endif

#ifdef ONEWIRE_SNMP_SUPPORT
const char ow_rom_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x03\x01";
#ifdef ONEWIRE_NAMING_SUPPORT
const char ow_name_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x03\x02";
#endif
const char ow_temp_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x03\x03";
const char ow_present_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x03\x04";
#endif

#ifdef TANKLEVEL_SUPPORT
const char tank_reaction_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x04";
#endif

#ifdef DHT_SNMP_SUPPORT
const char dht_general_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x05";

const char dht_polling_delay_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x05\x01";
const char dht_temp_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x05\x02";
const char dht_humid_obj_name[] PROGMEM = SNMP_OID_ETHERSEX "\x05\x03";
#endif

const struct snmp_reaction snmp_reactions[] PROGMEM = {
  {desc_obj_name, string_pgm_reaction, (void *) desc_value, NULL},
#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT)
  {uptime_reaction_obj_name, uptime_reaction, NULL, NULL},
#endif
  {contact_obj_name, string_pgm_reaction, (void *) contact_value, NULL},
  {hostname_reaction_obj_name, string_pgm_reaction, (void *) hostname_value, NULL},
  {location_obj_name, string_pgm_reaction, (void *) location_value, NULL},
#ifdef ADC_SUPPORT
  {adc_reaction_obj_name, adc_reaction, NULL, adc_next},
#ifdef ADC_VOLTAGE_SUPPORT
  {adc_volt_reaction_obj_name, adc_volt_reaction, NULL, adc_next},
  {adc_vref_reaction_obj_name, adc_vref_reaction, NULL, adc_next},
#endif
#endif
#ifdef ONEWIRE_SNMP_SUPPORT
  {ow_rom_reaction_obj_name, ow_rom_reaction, NULL, ow_next},
#ifdef ONEWIRE_NAMING_SUPPORT
  {ow_name_reaction_obj_name, ow_name_reaction, NULL, ow_next},
#endif
  {ow_temp_reaction_obj_name, ow_temp_reaction, NULL, ow_next},
  {ow_present_reaction_obj_name, ow_present_reaction, NULL, ow_next},
#endif
#ifdef TANKLEVEL_SUPPORT
  {tank_reaction_obj_name, tank_reaction, NULL, tank_next},
#endif
#ifdef DHT_SNMP_SUPPORT
  {dht_polling_delay_obj_name, dht_polling_delay_reaction, NULL, dht_next},
  {dht_temp_obj_name, dht_temp_reaction, NULL, dht_next},
  {dht_humid_obj_name, dht_humid_reaction, NULL, dht_next},
#endif
  {NULL, NULL, NULL, NULL}
};

#endif
