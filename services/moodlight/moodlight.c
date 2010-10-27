/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2010 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "services/moodlight/moodlight.h"
#include "core/eeprom.h"
#include "core/debug.h"
#include "core/bit-macros.h"

uint16_t moodlight_changegap = MOODLIGHT_CHANGEGAP_INIT;
uint16_t moodlight_counter;
uint8_t moodlight_mask;

/* Initialize moodlight */
void
moodlight_init (void)
{
	#if STELLA_START == stella_start_moodlight
	moodlight_mask = 0xff;
	#endif
}

/* Process recurring actions for moodlight */
void
moodlight_process (void)
{
	/* Only do something if any of the channels is selected
	 * to be 'moodlighted' and the moodlight counter reached
	 * the threshold value. */
	if (moodlight_mask && moodlight_counter > moodlight_changegap)
	{
		for (uint8_t i = 0; i < STELLA_CHANNELS; ++i)
		{
			if (moodlight_mask & _BV(i))
				stella_setValue(1, i, LO8(rand()));
		}

		moodlight_counter = 0;
	}
	else
		moodlight_counter ++;
}

/*
  -- Ethersex META --
  header(services/moodlight/moodlight.h)
	mainloop(moodlight_process)
	init(moodlight_init)
*/
