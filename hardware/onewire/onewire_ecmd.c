/*
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
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "core/util/fixedpoint.h"
#include "hardware/onewire/onewire.h"

#include "protocols/ecmd/ecmd-base.h"


/* parse an onewire rom address at cmd, write result to ptr */
int8_t
parse_ow_rom(char *cmd, ow_rom_code_t * rom)
{
  uint8_t *addr = rom->bytewise;
  uint8_t end;

  OW_DEBUG_ROM("called parse_ow_rom with string '%s'\n", cmd);

  /* read 8 times 2 hex chars into a byte */
  int ret = sscanf_P(cmd, PSTR("%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%c"),
                     addr + 0, addr + 1, addr + 2, addr + 3,
                     addr + 4, addr + 5, addr + 6, addr + 7, &end);

  OW_DEBUG_ROM("scanf returned %d\n", ret);

  if ((ret == 8) || ((ret == 9) && (end == ' ')))
  {
    OW_DEBUG_ROM("read rom %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 addr[0], addr[1], addr[2], addr[3],
                 addr[4], addr[5], addr[6], addr[7]);
    return 0;
  }

  return -1;
}


#ifdef ONEWIRE_ECMD_LIST_SUPPORT
int16_t
parse_cmd_onewire_list(char *cmd, char *output, uint16_t len)
{
#ifdef ONEWIRE_DS2502_SUPPORT
  int8_t list_type;
  while (*cmd == ' ')
    cmd++;
  switch (*cmd)
  {
    case 't':
      list_type = OW_LIST_TYPE_TEMP_SENSOR;
      break;
    case 'e':
      list_type = OW_LIST_TYPE_EEPROM;
      break;
    case '\0':
      list_type = OW_LIST_TYPE_ALL;
      break;
    default:
      return ECMD_ERR_PARSE_ERROR;
  }
  cmd++;                        /* for static bytes */
#endif
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)       /* indicator flag: real invocation:  0 */
  {
    cmd[0] = ECMD_STATE_MAGIC;  /* continuing call: 23 */
    cmd[1] = 0;                 /* counter for sensors in list */
  }
  uint8_t i = cmd[1];
  /* This is a special case: the while loop below printed a sensor which was
   * last in the list, so we still need to send an 'OK' after the sensor id */
  if (i >= OW_SENSORS_COUNT)
    return ECMD_FINAL_OK;

  int16_t ret = 0;
  do
  {
    if (ow_sensors[i].ow_rom_code.raw != 0)
    {
#ifdef ONEWIRE_DS2502_SUPPORT
      if (list_type == OW_LIST_TYPE_ALL ||
          (list_type == OW_LIST_TYPE_TEMP_SENSOR &&
           ow_temp_sensor(&ow_sensors[i].ow_rom_code)) ||
          (list_type == OW_LIST_TYPE_EEPROM &&
           ow_eeprom(&ow_sensors[i].ow_rom_code)))
      {
#endif
#ifdef ONEWIRE_NAMING_SUPPORT
        const char *name = "";
        if (ow_sensors[i].named)
          name = ow_sensors[i].name;

#endif
#ifdef ONEWIRE_ECMD_LIST_VALUES_SUPPORT
        char temperature[7];
        itoa_fixedpoint(ow_sensors[i].temp.val,
                        ow_sensors[i].temp.twodigits + 1, temperature,
                        sizeof(temperature));
#endif
        ret = snprintf_P(output, len, PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"
#ifdef ONEWIRE_NAMING_SUPPORT
                                           "\t%s"
#endif
#ifdef ONEWIRE_ECMD_LIST_VALUES_SUPPORT
                                           "\t%s"
#endif
#ifdef ONEWIRE_ECMD_LIST_POWER_SUPPORT
                                           "\t%d"
#endif
                         ), ow_sensors[i].ow_rom_code.bytewise[0],
                         ow_sensors[i].ow_rom_code.bytewise[1],
                         ow_sensors[i].ow_rom_code.bytewise[2],
                         ow_sensors[i].ow_rom_code.bytewise[3],
                         ow_sensors[i].ow_rom_code.bytewise[4],
                         ow_sensors[i].ow_rom_code.bytewise[5],
                         ow_sensors[i].ow_rom_code.bytewise[6],
                         ow_sensors[i].ow_rom_code.bytewise[7]
#ifdef ONEWIRE_NAMING_SUPPORT
                         , name
#endif
#ifdef ONEWIRE_ECMD_LIST_VALUES_SUPPORT
                         , temperature
#endif
#ifdef ONEWIRE_ECMD_LIST_POWER_SUPPORT
                         , ow_sensors[i].power
#endif
          );
#ifdef ONEWIRE_DS2502_SUPPORT
      }
#endif
    }
    i++;
  }
  while (ret == 0 && i < OW_SENSORS_COUNT);
  /* The while loop exited either because a sensor has been found or because
   * there is no sensor left, let's check for that */
  if (ret == 0)
  {
    /* => i has reached OW_SENSORS_COUNT */
    return ECMD_FINAL_OK;
  }
  /* else, ret is != 0 which means a sensor has been found and this functions
   * has to be called again to prevent a buffer overflow. save i to cmd[1] */
  cmd[1] = i;

  return ECMD_AGAIN(ret);
}
#endif /* ONEWIRE_ECMD_LIST_SUPPORT */

int16_t
parse_cmd_onewire_get(char *cmd, char *output, uint16_t len)
{
  ow_rom_code_t rom;
  int16_t ret;

  while (*cmd == ' ')
    cmd++;
  debug_printf("called onewire_get with: \"%s\"\n", cmd);

  ret = parse_ow_rom(cmd, &rom);
  if (ret < 0)
  {
#ifdef ONEWIRE_NAMING_SUPPORT
    ow_sensor_t *sensor = ow_find_sensor_name(cmd);
    if (sensor != NULL)
      memcpy(&rom, &sensor->ow_rom_code, sizeof(rom));
    else
#endif
      return ECMD_ERR_PARSE_ERROR;
  }
#ifdef ONEWIRE_DS18XX_SUPPORT
  if (ow_temp_sensor(&rom))
  {
    /* search the sensor... */
    ow_sensor_t *sensor = ow_find_sensor(&rom);
    if (sensor != NULL)
      ret =
        itoa_fixedpoint(sensor->temp.val, sensor->temp.twodigits + 1,
                        output, len);
    else
      ret = snprintf_P(output, len, PSTR("sensor not in list!"));

    return ECMD_FINAL(ret);
  }
#endif /* ONEWIRE_DS18XX_SUPPORT */
#ifdef ONEWIRE_DS2502_SUPPORT
  if (ow_eeprom(&rom))
  {
    debug_printf("reading mac\n");

    uint8_t mac[6];
    ret = ow_eeprom_read(&rom, mac);

    if (ret != 0)
    {
      debug_printf("mac read failed: %d\n", ret);
      return ECMD_ERR_READ_ERROR;
    }

    debug_printf("successfully read mac\n");

    debug_printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ret = snprintf_P(output, len,
                     PSTR("mac: %02x:%02x:%02x:%02x:%02x:%02x"),
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return ECMD_FINAL(ret);
  }
#endif /* ONEWIRE_DS2502_SUPPORT */

  debug_printf("unknown sensor type\n");
#ifdef TEENSY_SUPPORT
  strncpy_P(output, PSTR("unknown sensor type"), len);
  ret = strlen(output);
#else
  ret = snprintf_P(output, len, PSTR("unknown sensor type"));
#endif

  return ECMD_FINAL(ret);
}

/* naming support */
#ifdef ONEWIRE_NAMING_SUPPORT

int16_t
parse_cmd_onewire_name_set(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  char *romstr = strchr(cmd, ' ');
  if (romstr == NULL)
  {
    return ECMD_ERR_PARSE_ERROR;
  }
  *(romstr++) = 0;
  while (*romstr == ' ')
    romstr++;

  char *name = strchr(romstr, ' ');
  if (name == NULL)
  {
    return ECMD_ERR_PARSE_ERROR;
  }
  *(name++) = 0;
  while (*name == ' ')
    name++;

  uint8_t pos = atoi(cmd);
  if (pos >= OW_SENSORS_COUNT)
  {
    return ECMD_ERR_PARSE_ERROR;
  }

  ow_rom_code_t rom;
  if (parse_ow_rom(romstr, &rom) < 0 || rom.raw == 0)
  {
    return ECMD_ERR_PARSE_ERROR;
  }

  ow_sensors[pos].named = 1;
  ow_sensors[pos].ow_rom_code.raw = rom.raw;
  strncpy(ow_sensors[pos].name, name, OW_NAME_LENGTH);
  ow_sensors[pos].temp.val = 0;
  ow_sensors[pos].temp.twodigits = 0;
  ow_polling_interval = 1;

  for (uint8_t i = 0; i < OW_SENSORS_COUNT; i++)
  {
    if (i != pos && ow_sensors[i].ow_rom_code.raw == rom.raw)
    {
      ow_sensors[i].ow_rom_code.raw = 0;
      ow_sensors[i].named = 0;
      ow_sensors[i].present = 0;
      ow_sensors[i].temp.val = 0;
      ow_sensors[i].temp.twodigits = 0;
    }
  }

  /* perform bus discovery */
  ow_discover_interval = 1;

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_onewire_name_clear(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd == 0)
  {
    return ECMD_ERR_PARSE_ERROR;
  }

  uint8_t pos = atoi(cmd);
  if (pos > OW_SENSORS_COUNT)
  {
    return ECMD_ERR_PARSE_ERROR;
  }

  ow_sensors[pos].named = 0;

  /* perform bus discovery */
  ow_discover_interval = 1;

  return ECMD_FINAL_OK;
}


int16_t
parse_cmd_onewire_name_list(char *cmd, char *output, uint16_t len)
{
  int16_t ret;

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)       /* indicator flag: real invocation:  0 */
  {
    cmd[0] = ECMD_STATE_MAGIC;  /* continuing call: 23 */
    cmd[1] = 0;                 /* counter for sensors in list */
  }

  uint8_t i = cmd[1];

  /* This is a special case: the while loop below printed a sensor which was
   * last in the list, so we still need to send an 'OK' after the sensor id */
  if (i >= OW_SENSORS_COUNT)
  {
    return ECMD_FINAL_OK;
  }

  cmd[1] = i + 1;

  ow_rom_code_t rom;
  rom.raw = 0;
  const char *name = "";
  if (ow_sensors[i].named)
  {
    rom.raw = ow_sensors[i].ow_rom_code.raw;
    name = ow_sensors[i].name;
  }

  ret = snprintf_P(output, len,
                   PSTR("%d\t%02x%02x%02x%02x%02x%02x%02x%02x\t%s"),
                   i,
                   rom.bytewise[0],
                   rom.bytewise[1],
                   rom.bytewise[2],
                   rom.bytewise[3],
                   rom.bytewise[4],
                   rom.bytewise[5], rom.bytewise[6], rom.bytewise[7], name);

  /* set return value that the parser has to be called again */
  if (ret > 0)
    ret = ECMD_AGAIN(ret);

  return ECMD_FINAL(ret);
}

int16_t
parse_cmd_onewire_name_save(char *cmd, char *output, uint16_t len)
{
  ow_names_save();
  return ECMD_FINAL_OK;
}

#endif /* ONEWIRE_NAMING_SUPPORT */

/*
  -- Ethersex META --
  block([[Dallas_1-wire_Bus]])
  ecmd_ifdef(ONEWIRE_ECMD_LIST_SUPPORT)
    ecmd_feature(onewire_list, "1w list", , Return a list of the connected onewire devices)
  ecmd_endif()
  ecmd_ifdef(ONEWIRE_NAMING_SUPPORT)
    ecmd_feature(onewire_get, "1w get", DEVICE, Return temperature value of onewire device (provide 64-bit ID as 16-hex-digits or sensor name))
  ecmd_else()
    ecmd_feature(onewire_get, "1w get", DEVICE, Return temperature value of onewire device (provide 64-bit ID as 16-hex-digits))
  ecmd_endif()
  ecmd_ifdef(ONEWIRE_NAMING_SUPPORT)
    ecmd_feature(onewire_name_set, "1w name set", ID DEVICE NAME, Assign a name to/from an device address)
    ecmd_feature(onewire_name_clear, "1w name clear", ID, Delete a name mapping)
    ecmd_feature(onewire_name_list, "1w name list", , Return a list of mapped device names)
    ecmd_feature(onewire_name_save, "1w name save", , Save name mappings to EEPROM)
  ecmd_endif()
*/
