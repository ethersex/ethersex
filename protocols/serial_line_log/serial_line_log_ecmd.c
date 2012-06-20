/*
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
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "serial_line_log.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_SERIAL_LINE_LOG
# include "core/debug.h"
# define SLL_DEBUG(a...)  debug_printf("sll: " a)
#else
# define SLL_DEBUG(a...)
#endif

int16_t
parse_cmd_sll_get (char *cmd, char *output, uint16_t len)
{
  uint8_t sent_parts = 0;
  len = len - 1;

  /* Magic Marker Byte for the next calls of this function */
  if (cmd[0] == ECMD_STATE_MAGIC)
    {
      sent_parts = cmd[1];
      cmd[1]++;
    }
  else
    {
      cmd[0] = ECMD_STATE_MAGIC;
      /* If the timeout is reached we sent first a packet which is saying,
         that our device is offline */
      if (sll_data.timeout == 0)
	{
	  cmd[1] = 0;
	  len = sprintf_P (output, PSTR ("offline - "));
	  output[len] = ECMD_NO_NEWLINE;
	  return ECMD_AGAIN (len);
	}
      cmd[1] = 1;

    }

  /* Sensor data has timed out */
  uint8_t rest_len = sll_data.len - sent_parts * len;

  if (rest_len < len)
    {
      /* This is the final packet */
      memcpy (output, sll_data.data + sent_parts * len, rest_len);
      return ECMD_FINAL (rest_len);
    }
  else
    {
      memcpy (output, sll_data.data + sent_parts * len, len);
      output[len] = ECMD_NO_NEWLINE;
      return ECMD_AGAIN (len);
    }
}

/*
  -- Ethersex META --
  block([[Serial Line Logging|SLL]])
  ecmd_feature(sll_get, "sll get",,Request the logged data)
*/
