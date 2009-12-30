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

#include "protocols/ecmd/ecmd-base.h"

#ifdef MOTORCURTAIN_SUPPORT

#include "curtain.h"

int16_t parse_cmd_curtain_off (char *cmd, char *output, uint16_t len)
{
	motorCurtain_off();
	return ECMD_FINAL_OK;
}

int16_t parse_cmd_curtain_max (char *cmd, char *output, uint16_t len)
{
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), motorCurtain_getMax()));
}

int16_t parse_cmd_curtain_last (char *cmd, char *output, uint16_t len)
{
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), motorCurtain_getPosition()));
}

int16_t parse_cmd_curtain_set (char *cmd, char *output, uint16_t len)
{
	uint8_t value=0;
	uint16_t ret = 0; // must be 16 bit; because the answer length may be > 255
	if (cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u"),  &value);
	
	if (ret == 1)
	{
		motorCurtain_setPosition(value);
		//return ECMD_FINAL(snprintf_P(output, len, PSTR("%u"), stella_getValue(ch)));
		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

#endif  /* MOTORCURTAIN_SUPPORT */

/*
-- Ethersex META --
block([[MotorCurtain]] commands)
ecmd_feature(curtain_set, "curtain", VALUE, Set value of curtain. 0=closed..7=open. If you use fewer sensors, use the corrent value instead of 7. )
ecmd_feature(curtain_max, "curtainmax",, Return maximum position)
ecmd_feature(curtain_last, "curtainlast",, Return last known position)
ecmd_feature(curtain_off, "curtainoff",, Switch motor off)
*/
