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

/* Module description: the dmx effect module can alter dmx information to create animations and effects for
   headless setups or currently not animated devices */
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "core/debug.h"
#include "dmx-effect.h"
#include "services/dmx-storage/dmx_storage.h"

#ifdef DMX_EFFECT_SUPPORT


#ifdef DMX_EFFECT_RAINBOW
/* This function has been copied from the ethersex dmx module
   Author: Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 */
uint8_t rainbow_enabled = DMX_EFFECT_DISABLED;
uint8_t color_r, color_g, color_b = 0;
#define RAINBOW_DELAY 42
void dmx_effect_rainbow_colors(void)
{
	static uint8_t rainbow_step = 0;
	static uint16_t rainbow_delay = 0;
	if (rainbow_delay++ <= (RAINBOW_DELAY / DMX_STORAGE_CHANNELS)) return;
	rainbow_delay = 0;
	switch(rainbow_step) {
		case 0:
			if (color_g > 1) {
				color_g--;
				color_b++;
			} else {
				rainbow_step++;
			}
			break;
		case 1:
			if (color_r > 1) {
				color_r--;
				color_g++;
			} else {
				rainbow_step++;
			}
			break;
		case 2:
			if (color_b > 1) {
				color_b--;
				color_r++;
			} else {
				rainbow_step = 0;
			}
			break;
	}
	/*Copy colors to dmx storage*/
	for(uint16_t i=0;i<DMX_EFFECT_RAINBOW_CHANNELS+(DMX_EFFECT_RAINBOW_MARGIN*DMX_EFFECT_RAINBOW_CHANNELS);i++)
	{
#if (DMX_EFFECT_RAINBOW_MARGIN > 0)
		for(uint8_t j=0;j<DMX_EFFECT_RAINBOW_MARGIN;j++)
		{
			set_dmx_channel(DMX_EFFECT_RAINBOW_UNIVERSE, DMX_EFFECT_RAINBOW_OFFSET+i,DMX_EFFECT_RAINBOW_MARGIN_FILL);
			i++;
		}
#endif 
		if(i%(DMX_EFFECT_RAINBOW_MARGIN*3+3) == DMX_EFFECT_RAINBOW_MARGIN)
			set_dmx_channel(DMX_EFFECT_RAINBOW_UNIVERSE,DMX_EFFECT_RAINBOW_OFFSET+i,color_r);
		if(i%(DMX_EFFECT_RAINBOW_MARGIN*3+3) == 2*DMX_EFFECT_RAINBOW_MARGIN+1)
			set_dmx_channel(DMX_EFFECT_RAINBOW_UNIVERSE,DMX_EFFECT_RAINBOW_OFFSET+i,color_g);
		if(i%(DMX_EFFECT_RAINBOW_MARGIN*3+3) == 3*DMX_EFFECT_RAINBOW_MARGIN+2)
			set_dmx_channel(DMX_EFFECT_RAINBOW_UNIVERSE,DMX_EFFECT_RAINBOW_OFFSET+i,color_b);
	}
}
#endif /*Rainbow end*/
#ifdef DMX_EFFECT_RANDOM
uint8_t random_enabled = DMX_EFFECT_DISABLED;
uint8_t random_channels[DMX_EFFECT_RANDOM_CHANNELS] = {0};
void dmx_effect_random_colors_gen(void)
{
	#ifndef DMX_EFFECT_RANDOM_ALL
	uint8_t rnd_value=rand()%256;
	#endif
	for(uint8_t i=0;i<DMX_EFFECT_RANDOM_CHANNELS;i++)
	{
		#ifdef DMX_EFFECT_RANDOM_ALL
		uint8_t rnd_value=rand()%256;
		#endif
		random_channels[i]=rnd_value;
		
	}
}
void dmx_effect_random_colors_show()
{
	for(uint8_t i=0;i<DMX_EFFECT_RANDOM_CHANNELS+DMX_EFFECT_RANDOM_CHANNELS*DMX_EFFECT_RANDOM_MARGIN;i++)
	{
		#if (DMX_EFFECT_RAINBOW_MARGIN > 0)
		for(uint8_t j=0;j<DMX_EFFECT_RANDOM_MARGIN;j++)
		{
			set_dmx_channel(DMX_EFFECT_RANDOM_UNIVERSE, DMX_EFFECT_RANDOM_OFFSET+i,DMX_EFFECT_RANDOM_MARGIN_FILL);
			i++;
		}
		#endif 
		uint8_t tmp_value=get_dmx_channel(DMX_EFFECT_RANDOM_UNIVERSE,DMX_EFFECT_RANDOM_OFFSET+i);
		uint8_t target_value=random_channels[(i+1)/(DMX_EFFECT_RANDOM_MARGIN+1)-1];
		if(tmp_value > target_value)
			set_dmx_channel(DMX_EFFECT_RANDOM_UNIVERSE,DMX_EFFECT_RANDOM_OFFSET+i,tmp_value-1);
		else if (tmp_value < target_value)
			set_dmx_channel(DMX_EFFECT_RANDOM_UNIVERSE,DMX_EFFECT_RANDOM_OFFSET+i,tmp_value+1);
		//else == : we do nothing, everything's set

	}
}
#endif /*Random end*/
void dmx_effect_init()
{
	color_r = 255;
	color_g = 128;
	color_b = 0;
}
void dmx_effect_process()
{
	#ifdef DMX_EFFECT_RAINBOW
	if(rainbow_enabled == DMX_EFFECT_ENABLED)
		dmx_effect_rainbow_colors();
	#endif
	#ifdef DMX_EFFECT_RANDOM
	if(random_enabled == DMX_EFFECT_ENABLED)
	{
		static uint16_t counter=0;
		if(counter == 0)
		{
			//20 seconds have passed, generate new colors
			counter=100;
			dmx_effect_random_colors_gen();
		}
		counter--;
		dmx_effect_random_colors_show();
	}
	#endif
}
#endif
/*
   -- Ethersex META --
   header(services/dmx-effect/dmx-effect.h)
   init(dmx_effect_init)
   timer(2,dmx_effect_process())
*/
