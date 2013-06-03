/*
 *
 * Copyright(c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "services/tanklevel/tanklevel.h"
#include "hardware/adc/adc.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_tanklevel_show_params(char *cmd, char *output, uint16_t len)
{
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)	/* indicator flag: real invocation:  0 */
  {
    cmd[0] = ECMD_STATE_MAGIC;		/* continuing call: 23 */
    cmd[1] = 0;				/* counter for sensors in list*/
  }

  uint8_t i = cmd[1];
  cmd[1] = i + 1;

  switch(i)
  {
    case 0:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("sensor_offset: %d"),
        tanklevel_params_ram.sensor_offset));
    case 1:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("med_density: %d"),
        tanklevel_params_ram.med_density));
    case 2:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("ltr_per_m: %d"),
        tanklevel_params_ram.ltr_per_m));
    case 3:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("ltr_full: %d"),
        tanklevel_params_ram.ltr_full));
    case 4:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("raise_time: %d"),
        tanklevel_params_ram.raise_time));
    case 5:
      return ECMD_AGAIN(snprintf_P(output, len,
        PSTR("hold_time: %d"),
        tanklevel_params_ram.hold_time));
    default:
      return ECMD_FINAL_OK;
  }
}

int16_t
parse_cmd_tanklevel_set_param(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ') cmd++;

  char *valstr = strchr(cmd, ' ');
  if (valstr == NULL) {
    return ECMD_ERR_PARSE_ERROR;
  }
  *(valstr++) = 0;
  while (*valstr == ' ') valstr++;

  if (strcmp_P(cmd, PSTR("sensor_offset")) == 0) {
    int16_t val = atoi(valstr);
    if (val < -1000 || val > 1000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.sensor_offset = val;
  } else if (strcmp_P(cmd, PSTR("med_density")) == 0) {
    uint16_t val = atoi(valstr);
    if (val < 100 || val > 2000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.med_density = val;
  } else if (strcmp_P(cmd, PSTR("ltr_per_m")) == 0) {
    uint16_t val = atoi(valstr);
    if (val < 1 || val > 20000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.ltr_per_m = val;
  } else if (strcmp_P(cmd, PSTR("ltr_full")) == 0) {
    uint16_t val = atoi(valstr);
    if (val < 1 || val > 50000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.ltr_full = val;
  } else if (strcmp_P(cmd, PSTR("raise_time")) == 0) {
    uint16_t val = atoi(valstr);
    if (val < 1 || val > 3000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.raise_time = val;
  } else if (strcmp_P(cmd, PSTR("hold_time")) == 0) {
    uint16_t val = atoi(valstr);
    if (val < 1 || val > 3000) {
      return ECMD_ERR_PARSE_ERROR;
    }
    tanklevel_params_ram.hold_time = val;
  } else {
    return ECMD_ERR_PARSE_ERROR;
  }

  tanklevel_update_factor();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_tanklevel_save_param(char *cmd, char *output, uint16_t len)
{
  eeprom_save (tanklevel_params, &tanklevel_params_ram, sizeof(tanklevel_params_t));
  eeprom_update_chksum ();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_tanklevel_zero_sensor(char *cmd, char *output, uint16_t len)
{
  tanklevel_params_ram.sensor_offset = adc_get_voltage(TANKLEVEL_ADC_CHANNEL);

  return ECMD_FINAL(snprintf_P(output, len,
    PSTR("set sensor_offset to %d"),
    tanklevel_params_ram.sensor_offset));
}

int16_t
parse_cmd_tanklevel_start(char *cmd, char *output, uint16_t len)
{
  tanklevel_start();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_tanklevel_get(char *cmd, char *output, uint16_t len)
{
  clock_datetime_t dt;

  uint16_t level  = tanklevel_get();

  memset(&dt, 0, sizeof(dt));
  clock_localtime(&dt, tanklevel_get_ts());

  return ECMD_FINAL(snprintf_P(output, len,
    PSTR("%02d.%02d.%04d %02d:%02d:%02d\t%d\t%d"),
    dt.day,
    dt.month,
    dt.year + 1900,
    dt.hour,
    dt.min,
    dt.sec,
    level,
    tanklevel_params_ram.ltr_full));
}

/*
  -- Ethersex META --
  ecmd_feature(tanklevel_show_params, "tank param show", , show tanklevel parameters)
  ecmd_feature(tanklevel_set_param, "tank param set", PARAM VALUE, set tank parameter)
  ecmd_feature(tanklevel_save_param, "tank param save", , write tank parameter to EEPROM)
  ecmd_feature(tanklevel_zero_sensor, "tank zero", , probe sensor zero offset)
  ecmd_feature(tanklevel_start, "tank start", , start measure)
  ecmd_feature(tanklevel_get, "tank get", , get last value)
*/
