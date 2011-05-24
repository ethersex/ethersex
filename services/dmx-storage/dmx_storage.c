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
#include "config.h"
#include "core/debug.h"
#include "dmx_storage.h"
#ifdef DMX_STORAGE_SUPPORT
uint8_t dmx_universes[DMX_STORAGE_UNIVERSES][DMX_STORAGE_CHANNELS]={{0}};

struct dmx_slot dmx_universes_state[DMX_STORAGE_UNIVERSES][DMX_STORAGE_SLOTS]={{{DMX_UNCHANGED,DMX_SLOT_FREE}}};

/*This function searchs for a free slot an returns the id*/
int8_t dmx_storage_connect(uint8_t universe)
{
	if(universe < DMX_STORAGE_UNIVERSES)
	{
		for(uint8_t i=0;i<DMX_STORAGE_SLOTS;i++)
		{
			if(dmx_universes_state[universe][i].inuse == DMX_SLOT_USED)
				continue;
			else
			{
				#ifdef DMX_STORAGE_DEBUG
					debug_printf("DMX STOR: got new connection to universe %d, id is %d\n", universe, i);
				#endif
				dmx_universes_state[universe][i].inuse = DMX_SLOT_USED;
				return i;
			}
		}
	}
	return -1;
}
void dmx_storage_disconnect(uint8_t universe, int8_t slot)
{
	if(universe < DMX_STORAGE_UNIVERSES && slot < DMX_STORAGE_SLOTS && slot >= 0)
		dmx_universes_state[universe][slot].inuse = DMX_SLOT_FREE;
}
uint8_t get_dmx_channel(uint8_t universe,uint16_t channel)
{
	if(channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
		return dmx_universes[universe][channel];
	else
		return 0;
}
uint8_t get_dmx_channel_slot(uint8_t universe,uint16_t channel,int8_t slot)
{
	if(slot < DMX_STORAGE_SLOTS && slot >= 0)
		dmx_universes_state[universe][slot].state=DMX_UNCHANGED;
	if(channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
		return dmx_universes[universe][channel];
	else
		return 0;

}
uint8_t set_dmx_channel(uint8_t universe, uint16_t channel, uint8_t value)
{
	#ifdef DMX_STORAGE_DEBUG
		debug_printf("DMX STOR: set dmx_channels: Universe: %d channel: %d value: %d \n", universe, channel, value);
	#endif
	if(channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
	{
		dmx_universes[universe][channel]=value;
		for(uint8_t i=0;i<DMX_STORAGE_SLOTS;i++)
			dmx_universes_state[universe][i].state=DMX_NEWVALUES;
		return 0;
	}
	else
		return 1;
}
void set_dmx_channels(uint8_t *start, uint8_t universe,uint16_t len)
{
	/* if our input is bigger than our storage */
	if(len > DMX_STORAGE_CHANNELS)
		len=DMX_STORAGE_CHANNELS-1;
	#ifdef DMX_STORAGE_DEBUG
		debug_printf("DMX STOR: set dmx_channels: Universe: %d Length: %d \n", universe, len);
	#endif
	if(universe < DMX_STORAGE_UNIVERSES)
	{
		for(uint16_t i=0;i<len;i++)
		{
			dmx_universes[universe][i]=start[i];
			#ifdef DMX_STORAGE_DEBUG
				debug_printf("DMX STOR: Universe: %d chan: %d value %d \n", universe, i, dmx_universes[universe][i]);
			#endif
		}
		for(uint8_t i=0;i<DMX_STORAGE_SLOTS;i++)
			dmx_universes_state[universe][i].state=DMX_NEWVALUES;
	}
}

enum dmx_state get_dmx_universe_state(uint8_t universe, int8_t slot)
{
	if(universe < DMX_STORAGE_UNIVERSES && slot < DMX_STORAGE_SLOTS && slot >= 0)
		return dmx_universes_state[universe][slot].state;
	else
		return DMX_UNCHANGED;
}

#endif
