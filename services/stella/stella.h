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

typedef enum
{
  STELLA_SET_IMMEDIATELY,
  STELLA_SET_FADE,
  STELLA_SET_FLASHY,
  STELLA_SET_IMMEDIATELY_RELATIVE,
  STELLA_SET_FADESTEP,
  STELLA_GETALL = 255
} stella_set_function_e;

enum
{
  STELLA_FADE_NORMAL,
  STELLA_FADE_FLASHY,
  FADE_FUNC_LEN
};

typedef enum
{
  NOTHING_NEW,
  NEW_VALUES,
  UPDATE_VALUES
} stella_update_sync_e;

typedef struct stella_output_channels_struct
{
  uint8_t channel_count;
  uint8_t pwm_channels[STELLA_CHANNELS];
} stella_output_channels_struct_s;

typedef struct stella_port_with_portmask
{
  volatile uint8_t *port;
  uint8_t mask;
} stella_port_with_portmask_s;

typedef struct stella_timetable_entry
{
  uint8_t value;
  stella_port_with_portmask_s port;
  struct stella_timetable_entry *next;
} stella_timetable_entry_s;

#define stella_fade_func_0 0
#define stella_fade_func_1 1

typedef struct stella_timetable_struct
{
  stella_timetable_entry_s channel[STELLA_CHANNELS];
  stella_timetable_entry_s *head;
  stella_port_with_portmask_s port[STELLA_PORT_COUNT];
} stella_timetable_struct_s;

extern struct stella_timetable_struct *int_table;
extern struct stella_timetable_struct *cal_table;

/* to update i_* variables with their counterparts */
extern volatile stella_update_sync_e stella_sync;
extern volatile uint8_t stella_fade_counter;

extern uint8_t stella_portmask[STELLA_PORT_COUNT];
extern uint8_t stella_fade_step;
extern uint8_t stella_fade_func;

extern uint8_t stella_brightness[STELLA_CHANNELS];
extern uint8_t stella_fade[STELLA_CHANNELS];

/* stella.c */
void stella_init(void);
void stella_process(void);

/* Get a channel value.
 * Only call this function with a channel<STELLA_CHANNELS ! */
inline uint8_t
stella_getValue(const uint8_t channel)
{
  return stella_brightness[channel];
}

inline uint8_t
stella_getFadeValue(const uint8_t channel)
{
  return stella_fade[channel];
}

void stella_setValue(const stella_set_function_e func,
                     const uint8_t channel, const uint8_t value);
void stella_setFadestep(const uint8_t fadestep);
uint8_t stella_getFadestep(void);

void stella_loadFromEEROM(void);
void stella_loadFromEEROMFading(void);
void stella_storeToEEROM(void);

uint8_t stella_output_channels(void *target);

#endif /* STELLA_SUPPORT */

#endif /* STELLA_H */
