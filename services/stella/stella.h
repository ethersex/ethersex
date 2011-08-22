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

#ifndef STELLA_H
#define STELLA_H

#ifdef STELLA_SUPPORT

#ifdef STELLA_PINS_PORT2
	#define STELLA_PORT_COUNT 2
	#define STELLA_CHANNELS STELLA_PINS_PORT1+STELLA_PINS_PORT2
#else
	#define STELLA_PORT_COUNT 1
	#define STELLA_CHANNELS STELLA_PINS_PORT1
#endif

enum stella_set_function
{
  STELLA_SET_IMMEDIATELY,
  STELLA_SET_FADE,
  STELLA_SET_FLASHY,
  STELLA_SET_IMMEDIATELY_RELATIVE,
  STELLA_GETALL = 255
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
	uint8_t pwm_channels[STELLA_CHANNELS];
};

struct stella_port_with_portmask
{
	volatile uint8_t* port;
	uint8_t mask;
};

struct stella_timetable_entry
{
	uint8_t value;
	struct stella_port_with_portmask port;
	struct stella_timetable_entry* next;
};

#if STELLA_FADE_FUNCTION_INIT == stella_fade_func_0
#undef STELLA_FADE_FUNCTION_INIT
#define STELLA_FADE_FUNCTION_INIT 0
#else
#undef STELLA_FADE_FUNCTION_INIT
#define STELLA_FADE_FUNCTION_INIT 1
#endif

struct stella_timetable_struct
{
	struct stella_timetable_entry channel[STELLA_CHANNELS];
	struct stella_timetable_entry* head;
	struct stella_port_with_portmask port[STELLA_PORT_COUNT];
};

extern struct stella_timetable_struct* int_table;
extern struct stella_timetable_struct* cal_table;

/* to update i_* variables with their counterparts */
extern volatile enum stella_update_sync stella_sync;
extern volatile uint8_t stella_fade_counter;

extern uint8_t stella_portmask[STELLA_PORT_COUNT];
extern uint8_t stella_fade_step;
extern uint8_t stella_fade_func;

extern uint8_t stella_brightness[STELLA_CHANNELS];
extern uint8_t stella_fade[STELLA_CHANNELS];
/* stella.c */
void stella_init(void);
void stella_process(void);

uint8_t stella_getValue(const uint8_t channel);
void stella_setValue(const enum stella_set_function func, const uint8_t channel, const uint8_t value);
void stella_setFadestep(const uint8_t fadestep);
uint8_t stella_getFadestep();

void stella_loadFromEEROM(void);
void stella_loadFromEEROMFading(void);
void stella_storeToEEROM(void);

uint8_t stella_output_channels(void* target);

#endif  /* STELLA_SUPPORT */

#endif /* STELLA_H */
