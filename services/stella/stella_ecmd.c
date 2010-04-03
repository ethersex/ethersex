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

#ifdef STELLA_SUPPORT

#include "stella.h"

int16_t parse_cmd_stella_eeprom_store (char *cmd, char *output, uint16_t len)
{
	stella_storeToEEROM();
	return ECMD_FINAL_OK;
}

int16_t parse_cmd_stella_eeprom_load (char *cmd, char *output, uint16_t len)
{
	stella_loadFromEEROM();
	return ECMD_FINAL_OK;
}

int16_t parse_cmd_stella_fadestep (char *cmd, char *output, uint16_t len)
{
	uint8_t fadestep;
	int8_t ret = 0;
	if (cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u"), &fadestep);

	// get
	if (ret == 0)
	{
		return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), stella_fade_step));
	}
	// set
	else if (ret == 1)
	{
		stella_fade_step = fadestep;
		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_stella_channels (char *cmd, char *output, uint16_t len)
{
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), STELLA_CHANNELS));
}

int16_t parse_cmd_stella_channel (char *cmd, char *output, uint16_t len)
{
	char f=0;
	uint8_t ch=0;
	uint8_t value=0;
	uint16_t ret = 0; // must be 16 bit; because the answer length may be > 255
	if (cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u %u %c"), &ch, &value, &f);
	
	if (f=='s') f = 0; // set
	else if (f=='f') f = 1; // fade
	else if (f=='y') f = 2; // fade variant 2

	// return all channel values
	if (ret == 0)
	{
		// First return amount of channels with three bytes
		output[ret++] = ((uint8_t)STELLA_CHANNELS)/10 +48;
		output[ret++] = ((uint8_t)STELLA_CHANNELS)%10 +48;
		output[ret++] = '\n';
	  
		// return channel values
		for (ch = 0; ch<STELLA_CHANNELS; ret+=4,++ch)
		{
			value = stella_getValue(ch);
			output[ret+2] = value%10 +48;
			value /= 10;
			output[ret+1] = value%10 +48;
			value /= 10;
			output[ret+0] = value%10 +48;
			output[ret+3] = '\n';
		}
		// remove last newline character
		if (ret>0) --ret;
		return ret;
	}
	// return one channel value
	else if (ret == 1)
	{
		if (ch>=STELLA_CHANNELS) return ECMD_ERR_PARSE_ERROR;

		return ECMD_FINAL(snprintf_P(output, len, PSTR("%u"), stella_getValue(ch)));
	}
	// else set channel to value
	else if (ret >= 2)
	{
		if (ch>=STELLA_CHANNELS) return ECMD_ERR_PARSE_ERROR;
		stella_setValue(f, ch, value);

		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

#endif  /* STELLA_SUPPORT */

/*
-- Ethersex META --
block([[Stella_Light]] commands)
ecmd_feature(stella_eeprom_store, "stella store",, Store values in eeprom)
ecmd_feature(stella_eeprom_load, "stella load",, Load values from eeprom)
ecmd_feature(stella_channels, "channels",, Return stella channel size)
ecmd_feature(stella_channel, "channel", CHANNEL VALUE FUNCTION,Get/Set stella channel to value. Second and third parameters are optional. Function: You may use 's' for instant set, 'f' for fade and 'y' for flashy fade. )
ecmd_feature(stella_fadestep, "fadestep", FADESTEP, Get/Set stella fade step)
*/
