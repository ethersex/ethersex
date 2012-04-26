/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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
#include <avr/interrupt.h>
#include <string.h>
#include "config.h"
#include "to1.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_TO1
# include "core/debug.h"
# define TO1_DEBUG(a...)  debug_printf("msr1: " a)
#else
# define TO1_DEBUG(a...)
#endif

extern struct to1_data to1_sensors[TO1_SENSOR_COUNT];

int16_t 
parse_cmd_to1_get(char *cmd, char *output, uint16_t len) 
{
  uint8_t i;
  uint8_t next_sensor = 0;
  TO1_DEBUG("called to1 get ecmd %d\n", cmd[1]);

next_sensor:
  if (*cmd == ECMD_STATE_MAGIC) { /* Magic Marker Byte for the next calls of this function */
    next_sensor = cmd[1];
    cmd[1] ++;
  } else {
    cmd[0] = ECMD_STATE_MAGIC;
    cmd[1] = 1;
  }

  if (next_sensor >= TO1_SENSOR_COUNT) return ECMD_FINAL_OK;
  if (to1_sensors[next_sensor].timeout == 0) goto next_sensor;

  for (i = 0; i < 15; i++) {
    sprintf(output, "%02x", to1_sensors[next_sensor].data[i]);
    output += 2;
  }

  return ECMD_AGAIN(15 * 2);
}


/*
  -- Ethersex META --
  block([[Tanküberwachung|TO1]])
  ecmd_feature(to1_get, "to1 get",,Request data from sensor)
*/
