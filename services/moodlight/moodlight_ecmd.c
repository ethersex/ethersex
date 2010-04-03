/*
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "services/cron/cron.h"

#include "protocols/ecmd/ecmd-base.h"

#include "stella.h"

int16_t parse_cmd_moodlight (char *cmd, char *output, uint16_t len)
{
	uint8_t ch, onoff;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &ch, &onoff);
	
	if (ch>=STELLA_CHANNELS) return ECMD_ERR_PARSE_ERROR;
	
	// get
	if (ret == 1)
	{
		if (stella_moodlight_mask & _BV(ch))
			output[0] = 48 + 1;
			return 1;
		else
			output[0] = 48 + 0;
			return 1;
	}
	// set
	else if (ret == 2)
	{
		if (onoff)
			stella_moodlight_mask |= _BV(ch);
		else
			stella_moodlight_mask &= ~_BV(ch);
		return ECMD_FINAL_OK;
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

/*
-- Ethersex META --
block(MoodLight - StellaLight Expansion)
ecmd_feature(moodlight, "moodlight",CHANNEL ONOFF, Set CHANNEL moodlight on=1 or off=0. If no channel is given return on if channel CHANNEL is moodlighted)
*/

