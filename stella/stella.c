/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <stdint.h>
#include <string.h>
#include "../config.h"
#include "../eeprom.h"
#include "../debug.h"
#include "../cron/cron.h"
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
	//_TCCR2_PRESCALE |= _BV(CS20) | _BV(CS22);

	/* Normal PWM Mode, 64 Prescaler */
	_TCCR2_PRESCALE = 1<<CS22 | 0<<CS21 | 0<<CS20;

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

#ifdef CRON_SUPPORT
void cron_stella_callback(void* data)
{
	// data is of 2 byte size
	stella_newdata(data, 2);
}
#endif

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

	while (len > 1)
	{
		// bogus warning might be triggered -> ignore
		// "warning: comparison is always true due to limited range of data type"
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
		else if(*buf == STELLA_UNICAST_RESPONSE)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_UNICAST_RESPONSE\n");
			#endif
			struct stella_response_detailed_struct *response_packet = uip_appdata;

			/* identifier */
			response_packet->identifier[0] = 'S';
			response_packet->identifier[1] = STELLA_UNICAST_RESPONSE;
			response_packet->protocol_version = STELLA_PROTOCOL_VERSION;
			response_packet->channel_count = STELLA_PINS;

			/* copy the pwm channel values */
			memcpy(response_packet->pwm_channels, stella_color, sizeof(response_packet->pwm_channels));

			stella_net_unicast(sizeof(struct stella_response_detailed_struct));
		}
		#endif
		#ifdef STELLA_RESPONSE_BROADCAST
		else if(*buf == STELLA_BROADCAST_RESPONSE)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_BROADCAST_RESPONSE\n");
			#endif
			struct stella_response_detailed_struct *response_packet = uip_appdata;

			/* identifier */
			response_packet->identifier[0] = 'S';
			response_packet->identifier[1] = STELLA_BROADCAST_RESPONSE;
			response_packet->protocol_version = STELLA_PROTOCOL_VERSION;
			response_packet->channel_count = STELLA_PINS;

			/* copy the pwm channel values */
			memcpy(response_packet->pwm_channels, stella_color, sizeof(response_packet->pwm_channels));

			stella_net_broadcast(sizeof(struct stella_response_detailed_struct));
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
		#endif // STELLA_EEPROM
		#ifdef CRON_SUPPORT
		#ifdef STELLA_RESPONSE
		else if(*buf == STELLA_COUNT_CRONJOBS)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_COUNT_CRONJOBS\n");
			#endif
			/* identifier */
			((char*)uip_appdata)[0] = 'S';
			((char*)uip_appdata)[1] = STELLA_COUNT_CRONJOBS;
			/* data */
			((char*)uip_appdata)[2] = cron_jobs();
			/* send */
			stella_net_unicast(3);
		}
		else if(*buf == STELLA_GET_CRONJOBS)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_GET_CRONJOB\n");
			#endif
			/* identifier */
			((char*)uip_appdata)[0] = 'S';
			((char*)uip_appdata)[1] = STELLA_GET_CRONJOBS;

			// get cronjob data
			struct cron_event_t* job = cron_getjob(0);
			if (!job)
			{ // no jobs; send just the header
				((char*)uip_appdata)[2] = 0;
				stella_net_unicast(3);
			} else
			{
				uint8_t count = 0;
				uint8_t size = 3;
				while (job && size<UIP_APPDATA_SIZE && size<248)
				{
					/* data: first 7 bytes of the cronjob structure are of interest */
					memcpy(((char*)uip_appdata)+size, job, 7);
					size += 7;
					/* special case "stella": has two extra bytes for channel+value*/
					if (job->extradata && job->appid=='S')
					{
						((char*)uip_appdata)[size] = ((char*)job->extradata)[0];
						((char*)uip_appdata)[size+1] = ((char*)job->extradata)[1];
						size += 2;
					}
					++count;
					job = job->next;
				}
				((char*)uip_appdata)[2] = count;
				/* send */
				stella_net_unicast(size);
			}
		}
		else if(*buf == STELLA_GET_CRONJOB)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_GET_CRONJOB\n");
			#endif
			/* identifier */
			((char*)uip_appdata)[0] = 'S';
			((char*)uip_appdata)[1] = STELLA_GET_CRONJOB;
			uint8_t size = 2;

			// get cronjob data
			struct cron_event_t* job = cron_getjob(buf[1]);
			if (!job)
			{ // no job for the jobposition in buf[1]; send just the header
				stella_net_unicast(2);
			} else
			{
				/* data: first 7 bytes of the cronjob structure are of interest */
				memcpy(((char*)uip_appdata)+size, job, 7);
				size += 7;
				/* special case "stella": has two extra bytes for channel+value*/
				if (job->extradata && job->appid=='S')
				{
					((char*)uip_appdata)[size] = ((char*)job->extradata)[0];
					((char*)uip_appdata)[size+1] = ((char*)job->extradata)[1];
					size += 2;
				}
				/* send */
				stella_net_unicast(size);
			}
		}
		else if(*buf == STELLA_RM_CRONJOB)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_RM_CRONJOB\n");
			#endif
			// get cronjob data
			struct cron_event_t* job = cron_getjob(buf[1]);
			if (!job) continue;
			// remove it
			cron_jobrm(job);
		}
		#endif // STELLA_RESPONSE
		else if(*buf == STELLA_ADD_CRONJOB)
		{
			#ifdef DEBUG_STELLA
			debug_printf("STELLA_ADD_CRONJOB\n");
			#endif
			// we need cmd + 8 parameters
			// cmd, channel_cmd, value, minute, hour, day, month, dayofweek, times
			if (len<9) continue;

			// we allocate heap memory to save the target channel and value
			// this will be freed if the cron job gets removed
			struct ch_value_struct {
				uint8_t ch;
				uint8_t value;
			};
			struct ch_value_struct *data = malloc(sizeof(struct ch_value_struct));
			// we don't have memory space left on the heap -> abort
			if (!data) continue;

			data->ch = buf[1]; // channel
			data->value = buf[2]; // value
			cron_jobadd(cron_stella_callback, 'S', buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], data);

			// adjust len and buf
			len -= 9;
			buf += 9;
			continue;
		}
		#endif // CRON_SUPPORT

		// Decrease length, increase pointer address
		len -= 2;
		buf += 2;
	}

	if (flags & STELLA_FLAG_SORT)
		stella_sort (stella_color);
}
