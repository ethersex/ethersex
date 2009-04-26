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
#include "core/eeprom.h"
#include "core/debug.h"
#include "stella.h"
#include "stella_fading_functions.h"

uint8_t stella_brightness[STELLA_PINS];
uint8_t stella_fade[STELLA_PINS];

uint8_t stella_fade_func = STELLA_FADE_FUNCTION_INIT;
uint8_t stella_fade_step = STELLA_FADE_STEP_INIT;
volatile uint8_t stella_fade_counter = 0;
volatile enum stella_update_sync stella_sync;
uint8_t stella_portmask_neg;
#ifdef STELLA_MOODLIGHT
uint8_t stella_moodlight_threshold = STELLA_MOODLIGHT_THRESHOLD_INIT;
uint8_t stella_moodlight_mask = 0;
uint8_t stella_moodlight_counter = 0;
#endif

void stella_sort(void);

/* Initialize stella */
void
stella_init (void)
{
	int_table->head = 0;
	stella_sync = NOTHING_NEW;

	/* we need at least 64 ticks for the compare interrupt,
	 * therefore choose a prescaler of at least 64. */

	#ifdef STELLA_HIGHFREQ
	/* Normal PWM Mode, 64 Prescaler */
	_TCCR2_PRESCALE = _BV(CS22);
	debug_printf("Stella freq: %u Hz\n", F_CPU/64/(256*2));
	#else
	/* Normal PWM Mode, 128 Prescaler */
	_TCCR2_PRESCALE |= _BV(CS20) | _BV(CS22);
	debug_printf("Stella freq: %u Hz\n", F_CPU/128/(256*2));
	#endif

	/* Interrupt on overflow and CompareMatch */
	_TIMSK_TIMER2 |= _BV(TOIE2) | _BV(_OUTPUT_COMPARE_IE2);

	/* set stella port pins to output and save the negated port mask */
	stella_portmask_neg = (uint8_t)~(((1 << STELLA_PINS) - 1) << STELLA_OFFSET);
	STELLA_DDR = ((1 << STELLA_PINS) - 1) << STELLA_OFFSET;

	/* initialise the fade counter. Fading works like this:
	* -> decrement fade_counter
	* -> on zero, fade if neccessary
	* -> reset counter to fade_step
	*/
	stella_fade_counter = stella_fade_step;

	#if STELLA_START == stella_start_moodlight
	#ifdef STELLA_MOODLIGHT
	stella_moodlight_mask = 0xff;
	stella_moodlight_counter = STELLA_MOODLIGHT_THRESHOLD;
	#endif
	#endif
	#if STELLA_START == stella_start_eeprom
	stella_loadFromEEROMFading();
	#endif
	#if STELLA_START == stella_start_all
	memset(stella_fade, 1, sizeof(stella_fade));
	#endif

	stella_sort();
}

uint8_t
stella_output_channels(void* target)
{
	struct stella_output_channels_struct *buf = target;
	buf->channel_count = STELLA_PINS;
	memcpy(buf->pwm_channels, stella_brightness, 8);
	return sizeof(struct stella_output_channels_struct);
}

/* Process recurring actions for stella */
void
stella_process (void)
{
	/* the main loop is too fast, slow down */
	if (stella_fade_counter == 0)
	{
		uint8_t i;
		/* Moodlight functionality */
		#ifdef STELLA_MOODLIGHT
		/* Only do something if any of the channels is selected
		 * to be 'moodlighted' and the moodlight counter reached
		 * the threshold value. */
		if (stella_moodlight_mask && stella_moodlight_counter == stella_moodlight_threshold)
			for (i = 0; i < STELLA_PINS; ++i)
			{
				if (stella_moodlight_mask & _BV(i))
					stella_fade[i] = (uint8_t)rand()%256;
			}
		--stella_moodlight_counter;
		#endif

		/* Fade channels */
		for (i = 0; i < STELLA_PINS; ++i)
		{
			if (stella_brightness[i] == stella_fade[i])
				continue;

			stella_fade_funcs[stella_fade_func].p (i);

			stella_fade_counter = 1;
		}

		if (stella_fade_counter) stella_sync = UPDATE_VALUES;

		/* reset counter */
		stella_fade_counter = stella_fade_step;
	}

	/* sort if new values are available */
	if (stella_sync == UPDATE_VALUES)
		stella_sort();
}

void
stella_setValue(const enum stella_set_function func, const uint8_t channel, const uint8_t value)
{
	if (channel >= STELLA_PINS) return;

	switch (func)
	{
		case STELLA_SET_IMMEDIATELY:
			stella_brightness[channel] = value;
			stella_fade[channel] = value;
			stella_sync = UPDATE_VALUES;
			break;
		case STELLA_SET_FADE:
			stella_fade[channel] = value;
			break;
		case STELLA_SET_FLASHY:
			stella_brightness[channel] = value;
			stella_fade[channel] = 0;
			stella_sync = UPDATE_VALUES;
			break;
		case STELLA_SET_IMMEDIATELY_RELATIVE:
			stella_brightness[channel] += (int8_t)value;
			stella_fade[channel] += (int8_t)value;
			stella_sync = UPDATE_VALUES;
			break;
	}
}

/* Get a channel value.
 * Only call this function with a channel<STELLA_PINS ! */
inline uint8_t
stella_getValue(const uint8_t channel)
{
	return stella_brightness[channel];
}

void
stella_loadFromEEROMFading()
{
	eeprom_restore(stella_channel_values, stella_fade, 8);
}

void
stella_loadFromEEROM()
{
	eeprom_restore(stella_channel_values, stella_fade, 8);
	memcpy(stella_brightness, stella_fade, 8);
	stella_sync = UPDATE_VALUES;
}

void
stella_storeToEEROM()
{
	eeprom_save(stella_channel_values, stella_brightness, 8);
}

/* How to use:
 * Do not call this directly, but use "stella_sync = UPDATE_VALUES" instead.
 * Purpose:
 * Sort channels' brightness values from high to low (and the
 * interrupt time points from low to high), to be able to switch on
 * channels one after the other depending on their brightness level
 * and point in time.
 * Implementation details:
 * Use a "linked list" to avoid expensive memory copies. Main difference
 * to a real linked list is, that all elements are already preallocated
 * on the stack and are not allocated on demand.
 * The function directly writes to a "just calculated"-structure and if we
 * want new values in the pwm interrupt, we just have to swap pointers from
 * the "interrupt save"-structure to the "just calculated"-structure. (The
 * meaning of both structures changes, too, of course.)
 * Although we provide each channel in the structure with its neccessary
 * information such as portmask and brightness level, we will actually
 * ignore brightness levels of 0% and 100% due to not linking them to the linked list.
 * 100%-level channels are only switched on at the beginning of each
 * pwm cycle and not touched afterwards. Channels with same brightness
 * levels are merged together (their portmask at least).
 * */
inline void
stella_sort()
{
	struct stella_timetable_entry* current, *last;
	uint8_t i;

	cal_table->portmask = 0;
	cal_table->head = 0;

	for (i=0;i<STELLA_PINS;++i)
	{
		/* set current item */
		cal_table->channel[i].portmask = _BV(i+STELLA_OFFSET);
		cal_table->channel[i].value = 255 - stella_brightness[i];
		cal_table->channel[i].next = 0;
		#ifdef STELLA_GAMMACORRECTION
		if (i<100)
			cal_table->channel[i].gamma_wait_cycles = pgm_read_byte(stella_gamma[i]);
		else
			cal_table->channel[i].gamma_wait_cycles = 0;
		cal_table->channel[i].gamma_wait_counter = cal_table->channel[i].gamma_wait_cycles;
		#endif

		/* Sepcial cases: 0% brightness */
		if (stella_brightness[i] == 0) continue;

		//cal_table->portmask |= _BV(i+STELLA_OFFSET);

		if (stella_brightness[i] == 255)
		{
			cal_table->portmask |= _BV(i+STELLA_OFFSET);
			continue;
		}

		/* first item in linked list */
		if (!cal_table->head)
		{
			cal_table->head = &(cal_table->channel[i]);
			continue;
		}
		/* add to linked list with >=1 entries */
		current = cal_table->head; last = 0;
		while (current)
		{
			// same value as current item: do not add to linked list
			// but just update the portmask
			if (current->value == cal_table->channel[i].value)
			{
				//current->portmask &= (uint8_t)~_BV(i+STELLA_OFFSET);
				current->portmask |= _BV(i+STELLA_OFFSET);
				break;
			}
			// insert our new value at the head of the list
			else if (!last && current->value > cal_table->channel[i].value)
			{
				cal_table->channel[i].next = cal_table->head;
				cal_table->head = &(cal_table->channel[i]);
				break;
			}
			// insert our new value somewhere in betweem
			else if (current->value > cal_table->channel[i].value)
			{
				cal_table->channel[i].next = last->next;
				last->next = &(cal_table->channel[i]);
				break;
			}
			// reached the end of the linked list: just add our new entry
			else if (!current->next)
			{
				current->next = &(cal_table->channel[i]);
				break;
			}
			// else go to the next item in the linked list
			else
			{
				last = current;
				current = current->next;
			}
		}
	}

	#ifdef DEBUG_STELLA
	// debug out
	current = cal_table->head;
	i = 0;
	while (current)
	{
		i++;
		debug_printf("%u %s\n", current->value, debug_binary(current->portmask));
		current = current->next;
	}
	debug_printf("%s %u\n", debug_binary(stella_portmask_neg), stella_portmask_neg);
	#endif

	/* Allow the interrupt to actually apply the calculated values */
	stella_sync = NEW_VALUES;
}

/*
  -- Ethersex META --
  header(services/stella/stella.h)
  mainloop(stella_process)
  init(stella_init)
*/
