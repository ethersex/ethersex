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

#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "../config.h"
#include "../debug.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../cron/cron.h"
#include "ecmd.h"

#ifdef STELLA_SUPPORT

#include "../stella/stella.h"

int16_t parse_cmd_stella_channels (char *cmd, char *output, uint16_t len)
{
	return snprintf_P(output, len, PSTR("%d"), STELLA_PINS);
}

#ifdef STELLA_EEPROM
int16_t parse_cmd_stella_eeprom_store (char *cmd, char *output, uint16_t len)
{
	eeprom_save(stella_channel_values, stella_color, 8);
	return snprintf_P(output, len, PSTR("stored"));
}

int16_t parse_cmd_stella_eeprom_load (char *cmd, char *output, uint16_t len)
{
	eeprom_restore(stella_channel_values, stella_color, 8);
	memcpy(stella_fade, stella_color, 8);
	stella_sort(stella_color);
	return snprintf_P(output, len, PSTR("loaded"));
}
#endif

int16_t parse_cmd_stella_version (char *cmd, char *output, uint16_t len)
{
	return snprintf_P(output, len, PSTR("%d"), STELLA_PROTOCOL_VERSION);
}

#ifdef CRON_SUPPORT

int16_t parse_cmd_stella_cron (char *cmd, char *output, uint16_t len)
{
	struct ch_value_struct {
		uint8_t ch;
		uint8_t value;
	};
	// we allocate heap memory to save the target channel and value
	// this will be freed if the cron job gets removed
	struct ch_value_struct *data = malloc(sizeof(struct ch_value_struct));
	// we don't have memory space left on the heap -> abort
	if (!data) return -1;

	// parse user values
	int8_t minute, hour, day, month, dayofweek;
	uint8_t times;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u %i %i %i %i %i %u"), &(data->ch), &(data->value),
		&minute, &hour, &day, &month, &dayofweek, &times);

	if (ret>2 && data->ch<=STELLA_FLASH_COLOR_7)
	{
		if (ret<8) times = 0;
		if (ret<7) dayofweek = -1;
		if (ret<6) month = -1;
		if (ret<5) day = -1;
		if (ret<4) hour = -1;

		// add cron job
		cron_jobadd(cron_stella_callback, 'S', minute, hour, day, month, dayofweek, times, data);
		return snprintf_P(output, len, PSTR("stella cron"));
	} else {
		return -1;
	}
}
#endif

int16_t parse_cmd_stella_fadestep_set (char *cmd, char *output, uint16_t len)
{
	uint8_t fadestep;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &fadestep);

	if (ret == 1)
	{
		stella_fade_step = fadestep;
		return snprintf_P(output, len, PSTR("ok"));
	} else {
		return -1;
	}
}

int16_t parse_cmd_stella_fadestep_get (char *cmd, char *output, uint16_t len)
{
	return snprintf_P(output, len, PSTR("%d"), stella_fade_step);
}

int16_t parse_cmd_stella_channel_set (char *cmd, char *output, uint16_t len)
{
	struct ch_value_struct {
		uint8_t ch;
		uint8_t value;
	} ch_value;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &(ch_value.ch), &(ch_value.value));

	if (ret == 2 && ch_value.ch<STELLA_FLASH_COLOR_7)
	{
		stella_newdata((char*)&ch_value, 2);
		//stella_fade[ch]=value;
		return snprintf_P(output, len, PSTR("ok"));
	} else {
		return -1;
	}
}

int16_t parse_cmd_stella_channel_get (char *cmd, char *output, uint16_t len)
{
	uint8_t ch;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &ch);

	if (ret == 1 && ch<STELLA_PINS)
	{
		return snprintf_P(output, len, PSTR("value: %u"), stella_color[ch]);
	} else {
		return -1;
	}
}

#endif  /* STELLA_SUPPORT */
