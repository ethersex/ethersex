/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
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
#include <avr/pgmspace.h>
#include "config.h"
#include "core/debug.h"
#include "protocols/dmx/dmx.h"


#ifdef DMX_SUPPORT

int16_t
parse_cmd_dmx_set6chan(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint16_t startchan;
  uint16_t value_1, value_2, value_3, value_4, value_5, value_6;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u %u %u %u %u %u %u"),&startchan, &value_1, &value_2, &value_3, &value_4, &value_5, &value_6);
  if (ret != 7)
    return -1;

  if (startchan+6 > 512)
    return -1;

  if (dmx_txlen < startchan + 6)
    dmx_txlen = startchan + 6;

  dmx_prg = 0;
  dmx_set_chan_x(startchan, 6, (uint8_t []){value_1, value_2, value_3, value_4, value_5, value_6});
  dmx_index = 0;
  return 0;
}

int16_t
parse_cmd_dmx_fade(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  dmx_prg = 1;
  return 0;
}
#endif /* DMX_SUPPORT */
