/*
 * Copyright(C) 2006 Christian Dietrich <stettberger@dokucode.de>
 * Copyright(C) 2006 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2007 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright(C) 2008 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2009 David Gräff <david.graeff@web.de>
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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <string.h>

#include "../config.h"
#include "stella.h"
#include "../debug.h"

struct stella_timetable_struct timetable_1, timetable_2;
struct stella_timetable_struct* int_table = &timetable_1;
struct stella_timetable_struct* cal_table = &timetable_2;
struct stella_timetable_entry* current;

/* Use port mask to switch an pins if timetable says so and
 * set the next trigger point in time for the compare interrupt.
 * */
ISR(_VECTOR_OUTPUT_COMPARE2)
{
	if (!current) return;
	// Activate pins
	#ifdef STELLA_GAMMACORRECTION
	if (!current->gamma_wait_counter)
	{
		STELLA_PORT |= current->portmask;
		current->gamma_wait_counter = current->gamma_wait_cycles;
	}
	else
		--current->gamma_wait_counter;
	#else
	STELLA_PORT |= current->portmask;
	#endif
	current = current->next;

	if (current)
		_OUTPUT_COMPARE_REG2 = current->value;
}

/* If channel values have been updated (update_table is set) update all i_* variables.
 * Start the next pwm round. */
ISR(_VECTOR_OVERFLOW2)
{
	/* if new values are available, work with them */
	if(stella_sync == NEW_VALUES)
	{
		// swap pointer
		struct stella_timetable_struct* temp = int_table;
		int_table = cal_table;
		cal_table = temp;

		// reset update flag
		stella_sync = NOTHING_NEW;
	}

	/* count down the fade_timer counter. If zero, we process
    * stella_process from ethersex main */
	if (stella_fade_counter) stella_fade_counter--;

	/* Start the next pwm round */
	current = int_table->head;

	/* Deactivate pins except those used by the first timetable entry.
	 * Only deactivate pins if they belong to stella.
	 * Activate pins used by the first timetable entry. */
	STELLA_PORT = (STELLA_PORT & stella_portmask_neg) | int_table->portmask;

	if (current)
		_OUTPUT_COMPARE_REG2 = current->value;
}
