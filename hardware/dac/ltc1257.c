/*
 * Support for DAC LTC1257
 * real hardware access
 * Copyright (C) 2009 Meinhard Schneider <meini@meini.org>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <avr/io.h>
#include <util/delay.h>
#include "config.h"

#include "ltc1257.h"

uint16_t Delay = 0;

void
ltc1257_delay_get(uint16_t *d)
{
	LTC1257_CORE_DEBUG("ltc1257_delay_get begin.\n");

	*d = Delay;

	LTC1257_CORE_DEBUG("ltc1257_delay_get Delay = %u.\n", Delay);

	LTC1257_CORE_DEBUG("ltc1257_delay_get end.\n");
}

void
ltc1257_delay_set(uint16_t *d)
{
	LTC1257_CORE_DEBUG("ltc1257_delay_set begin.\n");

	Delay = *d;

	LTC1257_CORE_DEBUG("ltc1257_delay_set Delay = %u.\n", Delay);

	LTC1257_CORE_DEBUG("ltc1257_delay_set end.\n");
}

void
ltc1257_init()
{
	LTC1257_CORE_DEBUG("ltc1257_init begin.\n");

	/* CLK low */
	PIN_CLEAR(LTC1257_CLK);
	/* LOAD high */
	PIN_SET(LTC1257_LOAD);

	LTC1257_CORE_DEBUG("ltc1257_init end.\n");
}

void
ltc1257_set(uint16_t *value[], uint8_t num_values)
{
	uint8_t i;
	int8_t j;

	LTC1257_CORE_DEBUG("ltc1257_set begin.\n");

	/* first lower CLOCK */
	PIN_CLEAR(LTC1257_CLK);
	_delay_us(Delay);
	LTC1257_CORE_DEBUG("lower CLOCK.\n");

	for(i = 0; i < num_values; ++i)
	{
		LTC1257_CORE_DEBUG("processing value[%u] = %u.\n", i, *value[i]);

		/* output MSB first, LSB last */
		for(j = 11; j >= 0; --j)
		{
			if(*value[i] & (1 << j))
			{
				/* logical one */
				PIN_SET(LTC1257_DATA);
				LTC1257_CORE_DEBUG("processing value[%u], bit %u: 1.\n", i, 11-j);
			}
			else
			{
				/* logical zero */
				PIN_CLEAR(LTC1257_DATA);
				LTC1257_CORE_DEBUG("processing value[%u], bit %u: 0.\n", i, 11-j);
			}
			
			/* rise clock -> load bit */
			PIN_SET(LTC1257_CLK);
			_delay_us(Delay);
			PIN_CLEAR(LTC1257_CLK);
			_delay_us(Delay);
			LTC1257_CORE_DEBUG("processing value[%u], rise CLOCK, lower CLOCK.\n", i);
		}
	}
	/* lower LOAD -> load data to shift register */
	PIN_CLEAR(LTC1257_LOAD);
	_delay_us(Delay);
	PIN_SET(LTC1257_LOAD);
	_delay_us(Delay);
	LTC1257_CORE_DEBUG("lower LOAD, rise LOAD.\n");

	/* finally rise CLOCK */
	PIN_SET(LTC1257_CLK);
	LTC1257_CORE_DEBUG("rise CLOCK.\n");

	LTC1257_CORE_DEBUG("ltc1257_set end.\n");
}
