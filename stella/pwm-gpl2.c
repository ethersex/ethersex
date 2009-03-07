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

#define NEXT_COMPARE_INTERRUPT 0
#define PORT_MASK 1
#define NOTHING_NEW 0
#define NEW_VALUES 1
#define UPDATE_VALUES 2

/* for calculations only */
uint8_t timetable[STELLA_PINS][2];
uint8_t length;
uint8_t overflow_mask = 0;

/* interrupt save */
uint8_t i_timetable[STELLA_PINS][2];
uint8_t i_length;
uint8_t i_overflow_mask = 0;

/* count up from 0 to length */
uint8_t now = 0;

/* if set then update i_* variables with their counterparts */
volatile uint8_t update_table = NOTHING_NEW;

void
stella_pwm_init(void)
{
	/* Normal PWM Mode, 256 Prescaler */
	_TCCR2_PRESCALE |= _BV(CS21) | _BV(CS22);

	/* Interrupt on overflow and CompareMatch */
	_TIMSK_TIMER2 |= _BV(TOIE2) | _BV(_OUTPUT_COMPARE_IE2);

	/* set stella port pins to output */
	STELLA_DDR = ((1 << STELLA_PINS) - 1) << STELLA_OFFSET;
}

/* Use port mask to switch off pins if timetable says so and
   set the next trigger point in time for the compare interrupt */
ISR(_VECTOR_OUTPUT_COMPARE2)
{
	if(i_length)
	{
		STELLA_PORT &= ~i_timetable[now][PORT_MASK];
		if (++now < i_length)
			_OUTPUT_COMPARE_REG2 = i_timetable[now][NEXT_COMPARE_INTERRUPT];
	}
}

/* The timer overflow is the last point in time where the stella port is
   changed. Apply the overflow_mask to the stella port. If channel values changed
   (update_table is set) update all i_* variables. Start all over again with
   setting the compare interrupt */
ISR(_VECTOR_OVERFLOW2)
{
	/* update interrupt save variables only if the original variables changed */
	if(update_table == NEW_VALUES)
	{
		uint8_t i;
		for (i = 0; i < length; i ++)
		{
			i_timetable[i][PORT_MASK] = timetable[i][PORT_MASK];
			i_timetable[i][NEXT_COMPARE_INTERRUPT] = timetable[i][NEXT_COMPARE_INTERRUPT];
		}

		i_length = length;
		i_overflow_mask = overflow_mask;
		update_table = NOTHING_NEW;
	}

	/* Last update of the stella port for this round. But we already
	   use new values if available, to avoid flickering on 1->0 transit */
	STELLA_PORT = i_overflow_mask;

	/* Start the next pwm round */
	now = 0;
	_OUTPUT_COMPARE_REG2 = i_timetable[0][NEXT_COMPARE_INTERRUPT];
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
	length  = 0;
	overflow_mask = 0;
	
	uint8_t new_values, p_insert;

	/* Insert color brightness into timetable. Avoid duplicates.
	   Use dynamic length insertion sort for that purpose. */
	for (new_values = 0; new_values < STELLA_PINS; new_values++)
	{
		/* Do not take zero brightness values into consideration */
		if (color[new_values] == 0) continue;

		/* We definitly want this color to be on, at least on the last
		   point in time (timer overflow), therefore update 
                   the overflow_mask */
		overflow_mask |= (1 << (new_values + STELLA_OFFSET));

		/* If this is an always on color (brightness==255) 
                 * don't ever switch it off */
		if (color[new_values] == 255) continue;

		/* Find the right position */
		for (p_insert = 0; p_insert < length; p_insert++)
		{
			if (timetable[p_insert][NEXT_COMPARE_INTERRUPT] 
                            >= color[new_values]) 
                          break;
		}

		/* is this value already in timetable? Just update the port mask */
		if ((p_insert < length) 
                    && (timetable[p_insert][NEXT_COMPARE_INTERRUPT] == color[new_values]))
		{
			timetable[p_insert][PORT_MASK] |= _BV(new_values + STELLA_OFFSET);
			continue;
		}
		/* Increase length */
		++length;

		/* Move all successive elements one position */
		memmove(timetable[p_insert+1], timetable[p_insert],
                        2*sizeof(uint8_t)*(length-1-p_insert));
		/* Insert our new value */
		timetable[p_insert][NEXT_COMPARE_INTERRUPT] = color[new_values];
		timetable[p_insert][PORT_MASK] = _BV(new_values + STELLA_OFFSET);
        }

	/* Allow the interrupt to actually apply the calculated values */
	update_table = NEW_VALUES;
}
