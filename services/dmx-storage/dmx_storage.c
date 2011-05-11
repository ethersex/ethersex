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

/* Module description: This module stores 4 DMX Universes and provides R/W functions to other modules*/
#include <avr/io.h>
#include "config.h"
#include "core/debug.h"
#include "dmx_storage.h"
#ifdef DMX_STORAGE_SUPPORT
volatile uint8_t dmx_universes[DMX_STORAGE_UNIVERSES][DMX_STORAGE_CHANNELS]={{0}};

volatile uint8_t dmx_universes_state[DMX_STORAGE_UNIVERSES]={DMX_UNCHANGED};

uint8_t get_dmx_channel(uint16_t channel, uint8_t universe)
{
	/*TODO: This is a hack since this will only work with one module accessing this universe*/
	dmx_universes_state[universe]=DMX_UNCHANGED;
	if(channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
		return dmx_universes[universe][channel];
	else
		return 0;
}
void set_dmx_channel(uint16_t channel, uint8_t universe,uint8_t value)
{
	
	if(channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
	{
		dmx_universes[universe][channel]=value;
		dmx_universes_state[universe]=DMX_NEWVALUES;
	}
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
		dmx_universes_state[universe]=DMX_NEWVALUES;
	}
}

uint8_t get_dmx_universe_state(uint8_t universe)
{
	if(universe < DMX_STORAGE_UNIVERSES)
		return dmx_universes_state[universe];
}	
#endif
