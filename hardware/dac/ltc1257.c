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
#include "core/debug.h"
#include "protocols/syslog/syslog.h"

#include "ltc1257.h"

uint8_t Delay = 0;

void
ltc1257_delay_get(uint8_t *d)
{
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_get begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_get begin.\n");
	syslog_flush();
#endif
#endif

	*d = Delay;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_get Delay = %u.\n", Delay);
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_get Delay = %u.\n", Delay);
	syslog_flush();
#endif
#endif

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_get end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_get end.\n");
	syslog_flush();
#endif
#endif
}

void
ltc1257_delay_set(uint8_t *d)
{
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_set begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_set begin.\n");
	syslog_flush();
#endif
#endif

	Delay = *d;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_set Delay = %u.\n", Delay);
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_set Delay = %u.\n", Delay);
	syslog_flush();
#endif
#endif

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_delay_set end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_delay_set end.\n");
	syslog_flush();
#endif
#endif
}

void
ltc1257_init()
{
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_init begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_init begin.\n");
	syslog_flush();
#endif
#endif

	/* CLK low */
	PIN_CLEAR(LTC1257_CLK);
	/* LOAD high */
	PIN_SET(LTC1257_LOAD);

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_init end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_init end.\n");
	syslog_flush();
#endif
#endif
}

void
ltc1257_set(uint16_t *value[], uint8_t num_values)
{
	uint8_t i;
	int8_t j;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_set begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_set begin.\n");
	syslog_flush();
#endif
#endif

	/* first lower CLOCK */
	PIN_CLEAR(LTC1257_CLK);
	_delay_us(Delay);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: lower CLOCK.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: lower CLOCK.\n");
	syslog_flush();
#endif
#endif

	for(i = 0; i < num_values; ++i)
	{
#ifdef DEBUG_LTC1257
#ifdef DEBUG
		debug_printf("LTC1257: processing value[%u] = %u.\n", i, *value[i]);
#endif
#ifdef SYSLOG_SUPPORT
		syslog_sendf("LTC1257: processing value[%u] = %u.\n", i, *value[i]);
		syslog_flush();
#endif
#endif

		/* output MSB first, LSB last */
		for(j = 11; j >= 0; --j)
		{
			if(*value[i] & (1 << j))
			{
				/* logical one */
				PIN_SET(LTC1257_DATA);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
				debug_printf("LTC1257: processing value[%u], bit %u: 1.\n", i, 11-j);
#endif
#ifdef SYSLOG_SUPPORT
				syslog_sendf("LTC1257: processing value[%u], bit %u: 1.\n", i, 11-j);
				syslog_flush();
#endif
#endif
			}
			else
			{
				/* logical zero */
				PIN_CLEAR(LTC1257_DATA);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
				debug_printf("LTC1257: processing value[%u], bit %u: 0.\n", i, 11-j);
#endif
#ifdef SYSLOG_SUPPORT
				syslog_sendf("LTC1257: processing value[%u], bit %u: 1.\n", i, 11-j);
				syslog_flush();
#endif
#endif
			}
			
			/* rise clock -> load bit */
			PIN_SET(LTC1257_CLK);
			_delay_us(Delay);
			PIN_CLEAR(LTC1257_CLK);
			_delay_us(Delay);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
			debug_printf("LTC1257: processing value[%u], rise CLOCK, lower CLOCK.\n", i);
#endif
#ifdef SYSLOG_SUPPORT
			syslog_sendf("LTC1257: processing value[%u], rise CLOCK, lower CLOCK.\n", i);
			syslog_flush();
#endif
#endif
		}
	}
	/* lower LOAD -> load data to shift register */
	PIN_CLEAR(LTC1257_LOAD);
	_delay_us(Delay);
	PIN_SET(LTC1257_LOAD);
	_delay_us(Delay);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: lower LOAD, rise LOAD.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: lower LOAD, rise LOAD\n");
	syslog_flush();
#endif
#endif

	/* finally rise CLOCK */
	PIN_SET(LTC1257_CLK);
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: rise CLOCK.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: rise CLOCK.\n");
	syslog_flush();
#endif
#endif

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ltc1257_set end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ltc1257_set end.\n");
	syslog_flush();
#endif
#endif
}
