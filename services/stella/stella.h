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

#define STELLA_PROTOCOL_VERSION 3

#include "config.h"
#ifdef STELLA_SUPPORT

enum stella_colors
{
  STELLA_COLOR_0,
  STELLA_COLOR_1,
  STELLA_COLOR_2,
  STELLA_COLOR_3,
  STELLA_COLOR_4,
  STELLA_COLOR_5,
  STELLA_COLOR_6,
  STELLA_COLOR_7
};

enum stella_commands
{
  STELLA_SET_COLOR_0=0,
  STELLA_SET_COLOR_1,
  STELLA_SET_COLOR_2,
  STELLA_SET_COLOR_3,
  STELLA_SET_COLOR_4,
  STELLA_SET_COLOR_5,
  STELLA_SET_COLOR_6,
  STELLA_SET_COLOR_7,
  STELLA_FADE_COLOR_0,
  STELLA_FADE_COLOR_1,
  STELLA_FADE_COLOR_2,
  STELLA_FADE_COLOR_3,
  STELLA_FADE_COLOR_4,
  STELLA_FADE_COLOR_5,
  STELLA_FADE_COLOR_6,
  STELLA_FADE_COLOR_7,
  STELLA_FLASH_COLOR_0,
  STELLA_FLASH_COLOR_1,
  STELLA_FLASH_COLOR_2,
  STELLA_FLASH_COLOR_3,
  STELLA_FLASH_COLOR_4,
  STELLA_FLASH_COLOR_5,
  STELLA_FLASH_COLOR_6,
  STELLA_FLASH_COLOR_7,
  STELLA_SELECT_FADE_FUNC,
  STELLA_FADE_STEP,
  STELLA_ACK_RESPONSE,
  STELLA_UNICAST_GETVALUES,
  STELLA_BROADCAST_GETVALUES,
  STELLA_SAVE_TO_EEPROM,
  STELLA_LOAD_FROM_EEPROM,
  STELLA_COUNT_CRONJOBS,
  STELLA_GET_CRONJOBS,
  STELLA_RM_CRONJOB,
  STELLA_ADD_CRONJOB,
  STELLA_MOODLIGHT_MASK,
  STELLA_MOODLIGHT_THRESHOLD
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
void stella_cron_callback(void* data);
void stella_init(void);
void stella_process(void);

uint8_t stella_getValue(const uint8_t channel);
void stella_setValue(uint8_t channel_cmd, uint8_t value);

void stella_loadFromEEROM(void);
void stella_loadFromEEROMFading(void);
void stella_storeToEEROM(void);

#endif  /* STELLA_SUPPORT */
