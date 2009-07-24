/*
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
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

#include <stdint.h>
#include <string.h>
#include "core/eeprom.h"
#include "core/debug.h"

uint8_t moodlight_threshold = STELLA_MOODLIGHT_THRESHOLD_INIT;
uint8_t moodlight_mask = 0;
uint8_t moodlight_counter = 0;

/* Initialize moodlight */
void
moodlight_init (void)
{
	#if STELLA_START == stella_start_moodlight
	#ifdef STELLA_MOODLIGHT
	stella_moodlight_mask = 0xff;
	stella_moodlight_counter = STELLA_MOODLIGHT_THRESHOLD_INIT;
	#endif
	#endif

/* Process recurring actions for moodlight */
void
moodlight_process (void)
{
	/* the main loop is too fast, slow down */
	if (stella_moodlight_counter == 0)
	{
		uint8_t i;
		/* Only do something if any of the channels is selected
		 * to be 'moodlighted' and the moodlight counter reached
		 * the threshold value. */
		if (stella_moodlight_mask && stella_moodlight_counter == stella_moodlight_threshold)
		{
			for (i = 0; i < STELLA_PINS; ++i)
			{
				if (stella_moodlight_mask & _BV(i))
					stella_fade[i] = (uint8_t)rand()%256;
			}
		}
		--stella_moodlight_counter;
	}
}

/*
  -- Ethersex META --
  header(services/moodlight/moodlight.h)
	timer(300, moodlight_process())
	init(moodlight_init)
*/
