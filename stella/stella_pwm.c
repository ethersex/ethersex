/*
 Copyright(C) 2006 Christian Dietrich <stettberger@dokucode.de>
 Copyright(C) 2006 Jochen Roessner <jochen@lugrot.de>
 Copyright(C) 2007 Stefan Siegl <stesie@brokenpipe.de>
 Copyright(C) 2008 Jochen Roessner <jochen@lugrot.de>
 Copyright(C) 2009 David Gräff <david.graeff@web.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <string.h>

#include "../config.h"
#include "stella.h"
#include "../debug.h"

#define NEXT_COMPARE_INTERRUPT 0
#define PORT_MASK 1
#define GAMMA_CORRECTION 2
#define GAMMA_CORRECTION_COUNTER 3

#define NOTHING_NEW 0
#define NEW_VALUES 1
#define UPDATE_VALUES 2

/* for calculations only */
#ifdef STELLA_GAMMACORRECTION
uint8_t timetable[3][STELLA_PINS];
#else
uint8_t timetable[2][STELLA_PINS];
#endif
uint8_t length;
uint8_t stella_portmask_neg;

/* interrupt save */
#ifdef STELLA_GAMMACORRECTION
uint8_t i_timetable[4][STELLA_PINS];
#else
uint8_t i_timetable[2][STELLA_PINS];
#endif
uint8_t i_length;
uint8_t i_counter;

/* if set then update i_* variables with their counterparts */
volatile uint8_t update_table = NOTHING_NEW;

/* Get port state of an entry in the timetable.
   Used to realise gamma corrention if desired */
inline uint8_t portstate(volatile const uint8_t pos)
{
	// gamma correction
	#ifdef STELLA_GAMMACORRECTION
	// no correction defined
	if (!i_timetable[GAMMA_CORRECTION][pos])
		return i_timetable[PORT_MASK][pos];
	else
	// Count up gamma_counter. If it matches with gamma_corrention, output the port mask
	{
		if (i_timetable[GAMMA_CORRECTION_COUNTER][pos]==i_timetable[GAMMA_CORRECTION][pos])
		{
			i_timetable[GAMMA_CORRECTION_COUNTER][pos] = 0;
			return i_timetable[PORT_MASK][pos];
		}
		i_timetable[GAMMA_CORRECTION_COUNTER][pos]++;
		return 0;
	}
	#else // no correction
	return i_timetable[PORT_MASK][pos];
	#endif
}

/* Use port mask to switch on pins if timetable says so and
   set the next trigger point in time for the compare interrupt */
ISR(_VECTOR_OUTPUT_COMPARE2)
{
	// Activate pins
	STELLA_PORT |= portstate(i_counter);
	++i_counter;
	if (i_counter < i_length)
		_OUTPUT_COMPARE_REG2 = i_timetable[NEXT_COMPARE_INTERRUPT][i_counter];
}

/* If channel values have been updated (update_table is set) update all i_* variables.
 * Start the next pwm round. */
ISR(_VECTOR_OVERFLOW2)
{
	/* update interrupt save variables only if the original variables changed */
	if(update_table == NEW_VALUES)
	{
		// copy portmask and interrupt points
		#ifdef STELLA_GAMMACORRECTION
		memcpy(&(i_timetable[NEXT_COMPARE_INTERRUPT]), &(timetable[NEXT_COMPARE_INTERRUPT]), STELLA_PINS);
		memcpy(&(i_timetable[PORT_MASK]), &(timetable[PORT_MASK]), STELLA_PINS);
		memset(&(i_timetable[GAMMA_CORRECTION_COUNTER]), 0, STELLA_PINS);
		#else
		memcpy(i_timetable, timetable, STELLA_PINS*2);
		#endif
		i_length = length;

		// reset update flag
		update_table = NOTHING_NEW;
	}

	/* count down the fade_timer counter. If zero, we process
      stella_process from ethersex main */
	if (stella_fade_counter) stella_fade_counter--;

	/* Start the next pwm round */
	i_counter = 1;
	// Deactivate pins except those used by the first timetable entry
	// Only deactivate pins if they belong to stella, of course
	STELLA_PORT &= ( portstate(0) | stella_portmask_neg);
	// Activate pins used by the first timetable entry
	STELLA_PORT |= portstate(0);
	// Is there more than one entry? Set the next stop on our pwm tour.
	if (i_counter < i_length)
		_OUTPUT_COMPARE_REG2 = i_timetable[NEXT_COMPARE_INTERRUPT][i_counter];
}

void
stella_sort(uint8_t color[])
{
	/* Don't allow the interrupt to take over calculated values,
	   because we are already calculating new ones. Caution: This
	   is a design decision: If you update stella values too fast,
	   you won't ever see the changes! An update intervall of 5ms
	   should be ok though (depending on the processor load). */
	update_table = UPDATE_VALUES;

	/* There is always at least one entry, that defines the port
	   state */
	timetable[PORT_MASK][0] = 0;
	#ifdef STELLA_GAMMACORRECTION
	timetable[GAMMA_CORRECTION][0] = 0;
	#endif
	length  = 1;

	uint8_t color_index, p_insert, next_interrupt,i;

	/* Insert color brightness into timetable. Avoid duplicates.
	   Use dynamic length insertion sort for sorting. */
	for (color_index = 0; color_index < STELLA_PINS; ++color_index)
	{
		/* ignore brightness level 0 */
		if (color[color_index]==0) continue;

		/* calculate next interrupt point */
		next_interrupt = 255 - color[color_index];

		/* Find the right position */
		p_insert = 0;
		while ((p_insert < length) &&
			(timetable[NEXT_COMPARE_INTERRUPT][p_insert] < next_interrupt))
		{
			++p_insert;
		}

		/* is this value already in timetable? Just update the port mask */
		if ((p_insert < length) &&
			(timetable[NEXT_COMPARE_INTERRUPT][p_insert] == next_interrupt))
		{
			timetable[PORT_MASK][p_insert] |= _BV(color_index + STELLA_OFFSET);
			continue;
		}

		/* Increase length */
		++length;

		/* Move all successive elements one position */
		for (i=STELLA_PINS-1;i>p_insert;--i)
		{
			timetable[0][i] = timetable[0][i-1];
			timetable[1][i] = timetable[1][i-1];
			#ifdef STELLA_GAMMACORRECTION
			timetable[2][i] = timetable[2][i-1];
			#endif
		}

		/* Insert our new value */
		timetable[NEXT_COMPARE_INTERRUPT][p_insert] = next_interrupt;
		timetable[PORT_MASK][p_insert] = _BV(color_index + STELLA_OFFSET);

		// gamma correction
		#ifdef STELLA_GAMMACORRECTION
		// only correct dark brightness levels
		if (color[color_index]<20)
		{
			i_timetable[GAMMA_CORRECTION][p_insert] = (20-color[color_index]) /2;
		}
		#endif

	}

	#ifdef DEBUG_STELLA
	debug_printf("Stella sorting:\n");
	for (color_index = 0; color_index < length; ++color_index)
	{
		debug_printf("%u:%u %u\n", color_index, timetable[NEXT_COMPARE_INTERRUPT][color_index], timetable[PORT_MASK][color_index]);
	}
	#endif

	/* Allow the interrupt to actually apply the calculated values */
	update_table = NEW_VALUES;
}
