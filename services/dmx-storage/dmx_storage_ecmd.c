/*
 *
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#include <stdlib.h>
#include <string.h>
#include "core/debug.h"
#include "config.h"
#include "dmx_storage.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DMX_STORAGE_SUPPORT
int16_t parse_cmd_dmx_get_channel(char *cmd, char *output, uint16_t len)
{
	uint16_t ret=0, channel=0, universe=0;
	if (cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u %u"), &universe, &channel);
	if(ret == 2)
	{
		if (channel >= DMX_STORAGE_CHANNELS)
			return ECMD_ERR_PARSE_ERROR;
		if (universe >= DMX_STORAGE_UNIVERSES)
			return ECMD_ERR_PARSE_ERROR;
		itoa(get_dmx_channel(universe,channel), output, 10);
		return ECMD_FINAL(strlen(output));
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_dmx_set_channels(char *cmd, char *output, uint16_t len)
{
	uint16_t startchannel=0, universe=0, value=0, channelcounter=0,i=0,blankcounter=0;
	if (cmd[0]!=0) {
		sscanf_P(cmd, PSTR("%u %u"), &universe,&startchannel);
		if (startchannel >= DMX_STORAGE_CHANNELS)
			return ECMD_ERR_PARSE_ERROR;
		if (universe >= DMX_STORAGE_UNIVERSES)
			return ECMD_ERR_PARSE_ERROR;
		while(blankcounter<3)
		{	
			if(cmd[i] == ' ')
				blankcounter++;
			i++;
		}
		while (cmd[i]!='\0'){           //read and write all values
			sscanf_P(cmd+i, PSTR(" %u"),&value);
			if(set_dmx_channel(universe,startchannel+channelcounter,value))
				return ECMD_ERR_WRITE_ERROR;
			channelcounter++;
			do{                         //search for next space
				i++;
				if(cmd[i]=='\0') break;
			}while(cmd[i]!=' ');
		}

		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}
int16_t parse_cmd_dmx_channels(char *cmd, char *output, uint16_t len)
{
	itoa(DMX_STORAGE_CHANNELS, output, 10);
	return ECMD_FINAL(strlen(output));
}
int16_t parse_cmd_dmx_universes(char *cmd, char *output, uint16_t len)
{
	itoa(DMX_STORAGE_UNIVERSES, output, 10);
	return ECMD_FINAL(strlen(output));
}
int16_t parse_cmd_dmx_get_universe(char *cmd, char *output, uint16_t len)
{
	uint16_t ret=0, universe=0;
	uint8_t value=0;
	if (cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u"), &universe);
	if(ret == 1 && universe < DMX_STORAGE_UNIVERSES)
	{
		ret=0;
		static uint16_t chan = 0;
		value=get_dmx_channel(universe,chan);
		output[ret+2] = value%10 +48;
		value /= 10;
		output[ret+1] = value%10 +48;
		value /= 10;
		output[ret+0] = value%10 +48;
		ret+=3;
		if(chan < DMX_STORAGE_CHANNELS-1)
		{
			chan++;
			return ECMD_AGAIN(ret);
		}
		else
		{
			chan=0;
			return ECMD_FINAL(ret);
		}

	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

#endif
/*
   -- Ethersex META --
   block([[DMX_Storage]] commands)
   ecmd_feature(dmx_get_channel, "dmx get",, Return channel value) 
   ecmd_feature(dmx_set_channels, "dmx set",, Set channel values) 
   ecmd_feature(dmx_channels, "dmx channels",, Get channels per universe) 
   ecmd_feature(dmx_universes, "dmx universes",, Get universes) 
   ecmd_feature(dmx_get_universe, "dmx universe",, Get a whole universe) 
 */
