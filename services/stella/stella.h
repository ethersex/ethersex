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

#include "config.h"
#ifdef STELLA_SUPPORT

enum stella_set_function
{
  STELLA_SET_IMMEDIATELY,
  STELLA_SET_FADE,
  STELLA_SET_FLASHY
};

enum
{
	STELLA_FADE_NORMAL,
	STELLA_FADE_FLASHY,
	FADE_FUNC_LEN
};

enum stella_update_sync
{
	NOTHING_NEW,
	NEW_VALUES,
	UPDATE_VALUES
};

struct stella_output_channels_struct
{
	uint8_t channel_count;
	uint8_t pwm_channels[8];
};

struct stella_timetable_entry
{
	uint8_t portmask;
	uint8_t value;
	struct stella_timetable_entry* next;
	#ifdef STELLA_GAMMACORRECTION
	uint8_t gamma_wait_cycles;
	uint8_t gamma_wait_counter;
	#endif
};

#ifdef STELLA_GAMMACORRECTION
static const uint8_t stella_gamma[] PROGMEM =
{
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#endif

#if STELLA_FADE_FUNCTION_INIT == stella_fade_func_0
#undef STELLA_FADE_FUNCTION_INIT
#define STELLA_FADE_FUNCTION_INIT 0
#else
#undef STELLA_FADE_FUNCTION_INIT
#define STELLA_FADE_FUNCTION_INIT 1
#endif

struct stella_timetable_struct
{
	struct stella_timetable_entry channel[STELLA_PINS];
	struct stella_timetable_entry* head;
	uint8_t portmask;
};

extern struct stella_timetable_struct* int_table;
extern struct stella_timetable_struct* cal_table;

/* to update i_* variables with their counterparts */
extern volatile enum stella_update_sync stella_sync;
extern volatile uint8_t stella_fade_counter;

extern uint8_t stella_portmask_neg;
extern uint8_t stella_fade_step;
extern uint8_t stella_fade_func;

extern uint8_t stella_moodlight_mask;
extern uint8_t stella_moodlight_threshold;
extern uint8_t stella_moodlight_counter;

extern uint8_t stella_brightness[STELLA_PINS];
extern uint8_t stella_fade[STELLA_PINS];

/* stella.c */
void stella_init(void);
void stella_process(void);

uint8_t stella_getValue(const uint8_t channel);
void stella_setValue(const enum stella_set_function func, const uint8_t channel, const uint8_t value);

void stella_loadFromEEROM(void);
void stella_loadFromEEROMFading(void);
void stella_storeToEEROM(void);

uint8_t stella_output_channels(void* target);

#endif  /* STELLA_SUPPORT */
