/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2010 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "services/moodlight/moodlight.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "services/cron/cron.h"
#include "services/stella/stella.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_moodlight (char *cmd, char *output, uint16_t len)
{
	uint8_t ch, onoff;

	while(*cmd == ' ') cmd ++;
	char *p = cmd;
	while(*p && *p != ' ') p++;

  ch = atoi(cmd);
	if (ch >= STELLA_CHANNELS)
    return ECMD_ERR_PARSE_ERROR;

  if(!*p)
  {
		/* no second argument -> get */
		if (moodlight_mask & _BV(ch))
			output[0] = 48 + 1;
		else
			output[0] = 48 + 0;

		return 1;
	}

	onoff = atoi(p + 1);

	if (onoff)
		moodlight_mask |= _BV(ch);
	else
		moodlight_mask &= ~_BV(ch);

	return ECMD_FINAL_OK;
}

/*
-- Ethersex META --
block(MoodLight - StellaLight Expansion)
ecmd_feature(moodlight, "moodlight",CHANNEL ONOFF, Set CHANNEL moodlight on=1 or off=0. If no channel is given return on if channel CHANNEL is moodlighted)
*/

