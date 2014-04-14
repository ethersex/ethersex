/*
* ECMD-commands to handle reading DHT humidity & temp sensors
*
* Copyright (c) 2013-14 Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/util/fixedpoint.h"
#include "protocols/ecmd/ecmd-base.h"

#include "dht.h"
#include "dht_ecmd.h"

int16_t parse_cmd_dht_temp(char *cmd, char *output, uint16_t len)
{
  uint8_t sensor = 0;
  int ret = sscanf_P(cmd, PSTR("%hhu"), &sensor);
  return (sensor < dht_sensors_count ?
    ECMD_FINAL(itoa_fixedpoint(dht_sensors[sensor].temp,1,output)) :
    ECMD_ERR_PARSE_ERROR);
}

int16_t parse_cmd_dht_humid(char *cmd, char *output, uint16_t len)
{
  uint8_t sensor = 0;
  int ret = sscanf_P(cmd, PSTR("%hhu"), &sensor);
  return (sensor < dht_sensors_count ?
    ECMD_FINAL(itoa_fixedpoint(dht_sensors[sensor].humid,1,output)) :
    ECMD_ERR_PARSE_ERROR);
}

int16_t parse_cmd_dht_list(char *cmd, char *output, uint16_t len)
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
  if (i >= dht_sensors_count)
  {
    return ECMD_FINAL_OK;
  }

  cmd[1] = i + 1;

  ret = snprintf_P(output, len, PSTR("%d\t%S"), i, dht_sensors[i].name);

  /* set return value that the parser has to be called again */
  if (ret > 0)
    ret = ECMD_AGAIN(ret);

  return ECMD_FINAL(ret);
}

/*
  -- Ethersex META --
  block([[DHT]])
  ecmd_feature(dht_temp, "dht temp", [SENSORNUMBER], Return temperature of DHT sensor)
  ecmd_feature(dht_humid, "dht humid", [SENSORNUMBER], Return humidity of DHT sensor)
  ecmd_feature(dht_list, "dht list", , Return a list of mapped sensor names)
*/
