/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
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


#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "core/util/fixedpoint.h"
#include "hardware/onewire/onewire.h"
#include "protocols/mqtt/mqtt.h"

#define ONEWIRE_PUBLISH_NAME_FORMAT     ONEWIRE_MQTT_TOPIC "/%s"
#define ONEWIRE_PUBLISH_ROM_FORMAT      ONEWIRE_MQTT_TOPIC "/%02x%02x%02x%02x%02x%02x%02x%02x"
#define ONEWIRE_MQTT_RETAIN             false
#define TOPIC_LENGTH                    (sizeof(ONEWIRE_MQTT_TOPIC) + 17)

static const char publish_name_topic_format[] PROGMEM =
  ONEWIRE_PUBLISH_NAME_FORMAT;
static const char publish_rom_topic_format[] PROGMEM =
  ONEWIRE_PUBLISH_ROM_FORMAT;
uint8_t ow_mqtt_new;
uint8_t ow_act;

#ifdef DEBUG_OW_MQTT
#define OW_DEBUG_MQTT(s, ...) debug_printf("OW " s "\n", ## __VA_ARGS__)
#else
#define OW_DEBUG_MQTT(...) do { } while(0)
#endif

#ifdef ONEWIRE_POLLING_SUPPORT

void
onewire_poll_cb(void)
{
  if (!ow_mqtt_new)
    return;

  OW_DEBUG_MQTT("MQTT Poll");
  if (ow_sensors[ow_act].ow_rom_code.raw != 0)
  {
    char topic[TOPIC_LENGTH];
    char temperature[7];

    itoa_fixedpoint(ow_sensors[ow_act].temp.val,
                    ow_sensors[ow_act].temp.twodigits + 1, temperature,
                    sizeof(temperature));

    snprintf_P(topic, TOPIC_LENGTH, publish_rom_topic_format,
               ow_sensors[ow_act].ow_rom_code.bytewise[0],
               ow_sensors[ow_act].ow_rom_code.bytewise[1],
               ow_sensors[ow_act].ow_rom_code.bytewise[2],
               ow_sensors[ow_act].ow_rom_code.bytewise[3],
               ow_sensors[ow_act].ow_rom_code.bytewise[4],
               ow_sensors[ow_act].ow_rom_code.bytewise[5],
               ow_sensors[ow_act].ow_rom_code.bytewise[6],
               ow_sensors[ow_act].ow_rom_code.bytewise[7]);
    mqtt_construct_publish_packet(topic, temperature, strlen(temperature),
                                  ONEWIRE_MQTT_RETAIN);
    OW_DEBUG_MQTT("%s=%s", topic, temperature);

#ifdef ONEWIRE_NAMING_SUPPORT
    const char *name = "";
    if (ow_sensors[ow_act].named)
    {
      name = ow_sensors[ow_act].name;
      snprintf_P(topic, TOPIC_LENGTH, publish_name_topic_format, name);
      mqtt_construct_publish_packet(topic, temperature, strlen(temperature),
                                    ONEWIRE_MQTT_RETAIN);
      OW_DEBUG_MQTT("%s=%s", topic, temperature);
    }
#endif
  }
  if(++ow_act >= OW_SENSORS_COUNT)
  {
    ow_act=0;
    ow_mqtt_new = 0;
  }
}

static mqtt_callback_config_t mqtt_callback_config = {
  .connack_callback = NULL,
  .poll_callback = onewire_poll_cb,
  .close_callback = NULL,
  .publish_callback = NULL,
};

void
onewire_mqtt_init()
{
  OW_DEBUG_MQTT("MQTT Init");
  mqtt_register_callback(&mqtt_callback_config);
}

#else

void
onewire_mqtt_init()
{
  OW_DEBUG_MQTT("MQTT no init!");
}

#endif /* ONEWIRE_POLLING_SUPPORT ONEWIRE_DETECT_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/onewire/onewire_mqtt.h)
  net_init(onewire_mqtt_init)
*/
