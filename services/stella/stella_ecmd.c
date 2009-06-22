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

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "services/cron/cron.h"
#include "protocols/ecmd/speed_parser.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef STELLA_SUPPORT

#include "stella.h"

int16_t parse_cmd_stella_channels (char *cmd, char *output, uint16_t len)
{
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), STELLA_PINS));
}

int16_t parse_cmd_stella_eeprom_store (char *cmd, char *output, uint16_t len)
{
	stella_storeToEEROM();
	return ECMD_FINAL(snprintf_P(output, len, PSTR("stored")));
}

int16_t parse_cmd_stella_eeprom_load (char *cmd, char *output, uint16_t len)
{
	stella_loadFromEEROM();
	return ECMD_FINAL(snprintf_P(output, len, PSTR("loaded")));
}

#ifdef CRON_SUPPORT
#ifdef ECMD_SPEED_SUPPORT
int16_t parse_cmd_stella_cron (char *cmd, char *output, uint16_t len)
{
	struct ch_value_struct {
		uint8_t cmd;
		uint8_t ch;
		uint8_t value;
	};
	// we allocate heap memory to save the target channel and value
	// this will be freed if the cron job gets removed
	struct ch_value_struct *data = malloc(sizeof(struct ch_value_struct));
	// we don't have memory space left on the heap -> abort
	if (!data) return ECMD_ERR_PARSE_ERROR;

	data->cmd = ECMDS_SET_STELLA_FADE_COLOR;
	// parse user values
	int8_t minute, hour, day, month, dayofweek;
	uint8_t repeat;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u %i %i %i %i %i %u"), &(data->ch), &(data->value),
		&minute, &hour, &day, &month, &dayofweek, &repeat);

	if (ret>2 && data->ch<=STELLA_PINS)
	{
		if (ret<8) repeat = 0;
		if (ret<7) dayofweek = -1;
		if (ret<6) month = -1;
		if (ret<5) day = -1;
		if (ret<4) hour = -1;

		// add cron job
		cron_jobinsert(minute, hour, day, month, dayofweek, repeat, CRON_APPEND, sizeof(struct ch_value_struct), data);
		return ECMD_FINAL(snprintf_P(output, len, PSTR("stella cron")));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}
#endif
#endif

int16_t parse_cmd_stella_fadestep_set (char *cmd, char *output, uint16_t len)
{
	uint8_t fadestep;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &fadestep);

	if (ret == 1)
	{
		stella_fade_step = fadestep;
		return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

int16_t parse_cmd_stella_fadestep_get (char *cmd, char *output, uint16_t len)
{
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), stella_fade_step));
}

int16_t parse_cmd_stella_channel_set (char *cmd, char *output, uint16_t len)
{
	char f=0;
	uint8_t ch=0;
	uint8_t value=0;
	uint8_t ret = sscanf_P(cmd, PSTR("%d %d %c"), &ch, &value, &f);
	if (f=='s') f = 0;
	else if (f=='f') f = 1;
	else if (f=='y') f = 2;

	if (ret >= 2)
	{
		stella_setValue(f, ch, value);

		return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

int16_t parse_cmd_stella_channel_get (char *cmd, char *output, uint16_t len)
{
	uint8_t ch;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &ch);

	if (ret == 1 && ch<STELLA_PINS)
	{
		return ECMD_FINAL(snprintf_P(output, len, PSTR("value: %u"), stella_getValue(ch)));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

#ifdef STELLA_MOODLIGHT
int16_t parse_cmd_stella_moodlight_get (char *cmd, char *output, uint16_t len)
{
	uint8_t ch;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &ch);

	if (ret == 1 && ch<STELLA_PINS)
	{
		if (stella_moodlight_mask & _BV(ch))
			return ECMD_FINAL(snprintf_P(output, len, PSTR("on")));
		else
			return ECMD_FINAL(snprintf_P(output, len, PSTR("off")));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

int16_t parse_cmd_stella_moodlight_set (char *cmd, char *output, uint16_t len)
{
	uint8_t ch, onoff;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &ch, &onoff);

	if (ret == 2)
	{
		if (onoff)
			stella_moodlight_mask |= _BV(ch);
		else
			stella_moodlight_mask &= ~_BV(ch);
		return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
	} else {
		return ECMD_ERR_PARSE_ERROR;
	}
}

#endif

#endif  /* STELLA_SUPPORT */
