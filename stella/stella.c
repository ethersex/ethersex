/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#include <stdint.h>
#include <string.h>
#include "../config.h"
#include "../eeprom.h"
#include "stella.h"
#include "stella_net.h"
#include "stella_fading_functions.h"

uint8_t stella_color[STELLA_PINS];
uint8_t stella_fade[STELLA_PINS];

uint8_t stella_fade_func = 0;
uint8_t stella_fade_step = 10;
volatile uint8_t stella_fade_counter = 0;

/* Initialize stella */
void
stella_init (void)
{
	/* Normal PWM Mode, 128 Prescaler */
	_TCCR2_PRESCALE |= _BV(CS20) | _BV(CS22);

	/* Interrupt on overflow and CompareMatch */
	_TIMSK_TIMER2 |= _BV(TOIE2) | _BV(_OUTPUT_COMPARE_IE2);

	/* set stella port pins to output */
	STELLA_DDR = ((1 << STELLA_PINS) - 1) << STELLA_OFFSET;

	stella_fade_counter = stella_fade_step;

	/* load initial values from eeprom if menuconfig option is set */
	#ifdef STELLA_EEPROM
	eeprom_restore(stella_channel_values, stella_fade, 8);
		#ifndef STELLA_EEPROM_BOOT_FADE
		memcpy(stella_color, stella_fade, 8);
		#endif
	#endif

	#ifdef DEBUG_STELLA
	debug_printf("Stella init\n");
	#endif
}

/* Process recurring actions for stella */
void
stella_process (void)
{
	/* the main loop is too fast, slow down */
	if (stella_fade_counter > 0) return;

	uint8_t flag = 0;

	/* Fade channels */
	for (uint8_t i = 0; i < STELLA_PINS; ++i)
	{
		if (stella_color[i] == stella_fade[i])
			continue;

		stella_fade_funcs[stella_fade_func].p (i);
		flag = STELLA_FLAG_SORT;
	}

	if (flag == STELLA_FLAG_SORT)
		stella_sort (stella_color);

	/* reset counter */
	stella_fade_counter = stella_fade_step;
}

/* Process new channel data */
void
stella_newdata (unsigned char *buf, uint8_t len)
{
  uint8_t flags = 0;

  while (len > 1) {

	if(*buf >= STELLA_SET_COLOR_0 && *buf < (STELLA_SET_COLOR_7))
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_SET_COLOR\n");
		#endif
		stella_fade[*buf] = buf[1];
		stella_color[*buf] = buf[1];
		flags |= STELLA_FLAG_SORT;
	}
	else if(*buf >= STELLA_FADE_COLOR_0 && *buf < (STELLA_FADE_COLOR_7))
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_FADE_COLOR\n");
		#endif
		stella_fade[*buf & 0x07] = buf[1];
	}
	else if(*buf >= STELLA_FLASH_COLOR_0 && *buf < (STELLA_FLASH_COLOR_7))
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_FLASH_COLOR\n");
		#endif
		stella_fade[*buf & 0x07] = 0;
		stella_color[*buf & 0x07] = buf[1];
		flags |= STELLA_FLAG_SORT;
	}
	else if(*buf == STELLA_SELECT_FADE_FUNC)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_SELECT_FADE_FUNC\n");
		#endif
		stella_fade_func = buf[1];
	}
	else if(*buf == STELLA_FADE_STEP)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_FADE_STEP\n");
		#endif
		stella_fade_step = buf[1];
	}
	#ifdef STELLA_RESPONSE
	else if(*buf == STELLA_GET_PROTOCOL_VERSION)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_GET_PROTOCOL_VERSION\n");
		#endif
		stella_net_protocol_version();
	}
	else if(*buf == STELLA_UNICAST_RESPONSE)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_UNICAST_RESPONSE\n");
		#endif
		stella_net_unicast_response();
	}
	#endif
	#ifdef STELLA_RESPONSE_BROADCAST
	else if(*buf == STELLA_BROADCAST_RESPONSE)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_BROADCAST_RESPONSE\n");
		#endif
		stella_net_broadcast_response();
	}
	#endif
	#ifdef STELLA_RESPONSE_ACK
	else if(*buf == STELLA_ACK_RESPONSE)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_ACK_RESPONSE\n");
		#endif
		// yes, we interrupt the processing and send an ack
		stella_net_ack_response();
	}
	#endif
	#ifdef STELLA_EEPROM
	else if(*buf == STELLA_SAVE_TO_EEPROM)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_SAVE_TO_EEPROM\n");
		#endif
		eeprom_save(stella_channel_values, stella_color, 8);
	}
	else if(*buf == STELLA_LOAD_FROM_EEPROM)
	{
		#ifdef DEBUG_STELLA
		debug_printf("STELLA_LOAD_FROM_EEPROM\n");
		#endif
		eeprom_restore(stella_channel_values, stella_color, 8);
		memcpy(stella_fade, stella_color, 8);
		flags |= STELLA_FLAG_SORT;
	}
	#endif

	// Decrease length, increase pointer address
	len -= 2;
	buf += 2;
  }

  if (flags & STELLA_FLAG_SORT)
    stella_sort (stella_color);
}
