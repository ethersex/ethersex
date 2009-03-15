/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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
#include "../config.h"
#include "../debug.h"
#include "../eeprom.h"
#include "../bit-macros.h"
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
	return snprintf_P(output, len, PSTR("ok"));
}

int16_t parse_cmd_stella_eeprom_load (char *cmd, char *output, uint16_t len)
{
	eeprom_restore(stella_channel_values, stella_color, 8);
	memcpy(stella_fade, stella_color, 8);
	stella_sort(stella_color);
	return snprintf_P(output, len, PSTR("ok"));
}
#endif

int16_t parse_cmd_stella_version (char *cmd, char *output, uint16_t len)
{
	return snprintf_P(output, len, PSTR("%d"), STELLA_PROTOCOL_VERSION);
}

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
	uint8_t ch, value;
	uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &ch, &value);

	if (ret == 2 && ch<STELLA_PINS)
	{
		stella_fade[ch]=value;
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
