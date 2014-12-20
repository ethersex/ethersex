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
#include "services/dmx-storage/dmx_storage.h"

#ifdef 	STELLA_USE_CIE1931
#include "cie1931.h"
#endif

#define stella_vslow 0
#define stella_slow 1
#define stella_normal 2
#define stella_fast 3

#define stella_start_zero 0
#define stella_start_all 1
#define stella_start_eeprom 2

uint8_t stella_brightness[STELLA_CHANNELS];
uint8_t stella_fade[STELLA_CHANNELS];

uint8_t stella_fade_func = STELLA_FADE_FUNCTION_INIT;
uint8_t stella_fade_step = STELLA_FADE_STEP_INIT;
volatile uint8_t stella_fade_counter = 0;

volatile stella_update_sync_e stella_sync;
uint8_t stella_portmask[STELLA_PORT_COUNT];

struct stella_timetable_struct timetable_1, timetable_2;
struct stella_timetable_struct *int_table;
struct stella_timetable_struct *cal_table;
#ifdef DMX_STORAGE_SUPPORT
uint8_t stella_dmx_conn_id;
#endif
static void stella_sort(void);


void
stella_init(void)
{
  int_table = &timetable_1;
  cal_table = &timetable_2;
  cal_table->head = 0;

  stella_sync = NOTHING_NEW;

  /* set stella port pins to output and save the port mask */
  stella_portmask[0] = ((1 << STELLA_PINS_PORT1) - 1) << STELLA_OFFSET_PORT1;
  STELLA_DDR_PORT1 |= stella_portmask[0];
  cal_table->port[0].port = &STELLA_PORT1;
  cal_table->port[0].mask = 0;
#ifdef STELLA_PINS_PORT2
  stella_portmask[1] = ((1 << STELLA_PINS_PORT2) - 1) << STELLA_OFFSET_PORT2;
  STELLA_DDR_PORT2 |= stella_portmask[1];
  cal_table->port[0].port = &STELLA_PORT2;
  cal_table->port[1].mask = 0;
#endif

  /* initialise the fade counter. Fading works like this:
   * -> decrement fade_counter
   * -> on zero, fade if neccessary
   * -> reset counter to fade_step
   */
  stella_fade_counter = stella_fade_step;

#if !defined(TEENSY_SUPPORT) && STELLA_START == stella_start_eeprom
  stella_loadFromEEROMFading();
#endif
#if STELLA_START == stella_start_all
  memset(stella_fade, 255, sizeof(stella_fade));
#endif
#if STELLA_START == stella_start_zero
  memset(stella_fade, 0, sizeof(stella_fade));
#endif

  stella_sort();

  /* we need at least 64 ticks for the compare interrupt,
   * therefore choose a prescaler of at least 64. */
#if STELLA_FREQ == stella_vslow
  STELLA_TC_PRESCALER_1024;
  debug_printf("Stella freq: %u Hz\n", F_CPU / 1024 / (256 * 2));
#elif STELLA_FREQ == stella_slow
  STELLA_TC_PRESCALER_256;
  debug_printf("Stella freq: %u Hz\n", F_CPU / 256 / (256 * 2));
#elif STELLA_FREQ == stella_normal
  STELLA_TC_PRESCALER_128;
  debug_printf("Stella freq: %u Hz\n", F_CPU / 128 / (256 * 2));
#elif STELLA_FREQ == stella_fast
  STELLA_TC_PRESCALER_64;
  debug_printf("Stella freq: %u Hz\n", F_CPU / 64 / (256 * 2));
#endif

  STELLA_TC_INT_OVERFLOW_ON;
  STELLA_TC_INT_COMPARE_ON;

#ifdef DMX_STORAGE_SUPPORT
  /* Setup DMX-Storage Connection */
  stella_dmx_conn_id = dmx_storage_connect(STELLA_UNIVERSE);
#endif
}

uint8_t
stella_output_channels(void *target)
{
  stella_output_channels_struct_s *buf = target;
  buf->channel_count = STELLA_CHANNELS;
  memcpy(buf->pwm_channels, stella_brightness, STELLA_CHANNELS);
  return sizeof(stella_output_channels_struct_s);
}

void
stella_process(void)
{
#ifdef DMX_STORAGE_SUPPORT
  if (get_dmx_slot_state(STELLA_UNIVERSE, stella_dmx_conn_id) ==
      DMX_NEWVALUES)
  {
    uint8_t mode =
      get_dmx_channel_slot(STELLA_UNIVERSE, STELLA_UNIVERSE_OFFSET,
                           stella_dmx_conn_id);
    for (uint8_t i = 0; i < STELLA_CHANNELS; i++)
    {
      stella_setValue(mode, i,
                      get_dmx_channel_slot(STELLA_UNIVERSE,
                                           STELLA_UNIVERSE_OFFSET + i + 1,
                                           stella_dmx_conn_id));
    }
  }
#endif
  /* the main loop is too fast, slow down */
  if (stella_fade_counter == 0)
  {
    uint8_t i;
    /* Fade channels. stella_fade_counter is 0 currently. Set to 1
     * if fading changed a channel brigthness value */
    for (i = 0; i < STELLA_CHANNELS; ++i)
    {
      if (stella_brightness[i] == stella_fade[i])
        continue;

      stella_fade_funcs[stella_fade_func].p(i);

      stella_fade_counter = 1;
    }

    if (stella_fade_counter)
      stella_sync = UPDATE_VALUES;

    /* reset counter */
    stella_fade_counter = stella_fade_step;
  }

  /* sort if new values are available */
  if (stella_sync == UPDATE_VALUES)
    stella_sort();
}

void
stella_setValue(const stella_set_function_e func, const uint8_t channel,
                const uint8_t value)
{
#ifdef DEBUG_STELLA
  debug_printf("STELLA: channel: %d of %d\n", channel + 1, STELLA_CHANNELS);
#endif
  if (channel >= STELLA_CHANNELS)
    return;

  switch (func)
  {
    case STELLA_SET_IMMEDIATELY:
      stella_brightness[channel] = value;
      stella_fade[channel] = value;
      stella_sync = UPDATE_VALUES;
#ifdef DEBUG_STELLA
      debug_printf("STELLA: set immediately  value: %d\n", value);
#endif
      break;
    case STELLA_SET_FADE:
      stella_fade[channel] = value;
#ifdef DEBUG_STELLA
      debug_printf("STELLA: set fadeing value: %d\n", value);
#endif
      break;
    case STELLA_SET_FLASHY:
      stella_brightness[channel] = value;
      stella_fade[channel] = 0;
      stella_sync = UPDATE_VALUES;
#ifdef DEBUG_STELLA
      debug_printf("STELLA: set flashy value: %d\n", value);
#endif
      break;
    case STELLA_SET_IMMEDIATELY_RELATIVE:
      stella_brightness[channel] += (int8_t) value;
      stella_fade[channel] += (int8_t) value;
      stella_sync = UPDATE_VALUES;
#ifdef DEBUG_STELLA
      debug_printf("STELLA: set imidiatley relative value: %d\n", value);
#endif
      break;
    default:
#ifdef DEBUG_STELLA
      debug_printf("STELLA: What? you set to %d\n", func);
#endif
      break;
  }
}

void
stella_setFadestep(const uint8_t fadestep)
{
  stella_fade_step = fadestep;
}

uint8_t
stella_getFadestep(void)
{
  return stella_fade_step;
}

#ifndef TEENSY_SUPPORT
void
stella_loadFromEEROMFading(void)
{
  eeprom_restore(stella_channel_values, stella_fade, STELLA_CHANNELS);
}
#endif

#ifndef TEENSY_SUPPORT
void
stella_loadFromEEROM(void)
{
  eeprom_restore(stella_channel_values, stella_fade, STELLA_CHANNELS);
  memcpy(stella_brightness, stella_fade, STELLA_CHANNELS);
  stella_sync = UPDATE_VALUES;
}
#endif

#ifndef TEENSY_SUPPORT
void
stella_storeToEEROM(void)
{
  eeprom_save(stella_channel_values, stella_brightness, STELLA_CHANNELS);
  eeprom_update_chksum();
}
#endif

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
 * 100%-level channels are switched on at the beginning of each
 * pwm cycle and not touched afterwards. Channels with same brightness
 * levels are merged together (their portmask at least).
 * */
static void
stella_sort()
{
  stella_timetable_entry_s *current, *last;
  uint8_t i;

  cal_table->head = 0;
  cal_table->port[0].mask = 0;
  cal_table->port[0].port = &STELLA_PORT1;
#ifdef STELLA_PINS_PORT2
  cal_table->port[1].mask = 0;
  cal_table->port[1].port = &STELLA_PORT2;
#endif

  for (i = 0; i < STELLA_CHANNELS; ++i)
  {
    /* set data of channel i */
    cal_table->channel[i].port.mask = _BV(i + STELLA_OFFSET_PORT1);
    cal_table->channel[i].port.port = &STELLA_PORT1;
#ifdef STELLA_PINS_PORT2
    if (i >= STELLA_PINS_PORT1)
    {
      cal_table->channel[i].port.mask =
        _BV((i - STELLA_PINS_PORT1) + STELLA_OFFSET_PORT2);
      cal_table->channel[i].port.port = &STELLA_PORT2;
    }
#endif
#ifdef STELLA_USE_CIE1931
		cal_table->channel[i].value = 255 - pgm_read_byte_near(cie_luminance_8bit + stella_brightness[i]);
#else
    cal_table->channel[i].value = 255 - stella_brightness[i];
#endif
    cal_table->channel[i].next = 0;

    /* Special case: 0% brightness (Don't include this channel!) */
    if (cal_table->channel[i].value == 255)
      continue;

    //cal_table->portmask |= _BV(i+STELLA_OFFSET);

    /* Special case: 100% brightness (Merge pwm cycle start masks! Don't include this channel!) */
    if (cal_table->channel[i].value == 0)
    {
#ifdef STELLA_PINS_PORT2
      if (i >= STELLA_PINS_PORT1)
        cal_table->port[1].mask |=
          _BV((i - STELLA_PINS_PORT1) + STELLA_OFFSET_PORT2);
      else
        cal_table->port[0].mask |= _BV(i + STELLA_OFFSET_PORT1);
#else
      cal_table->port[0].mask |= _BV(i + STELLA_OFFSET_PORT1);
#endif
      continue;
    }

    /* first item in linked list */
    if (!cal_table->head)
    {
      cal_table->head = &(cal_table->channel[i]);
      continue;
    }
    /* add to linked list with >=1 entries */
    current = cal_table->head;
    last = 0;
    while (current)
    {
      // same value as current item: do not add to linked list
      // but just update the portmask (DO THIS ONLY IF BOTH CHANNELS OPERATE ON THE SAME PORT)
      if (current->value == cal_table->channel[i].value &&
          current->port.port == cal_table->channel[i].port.port)
      {
#ifdef STELLA_PINS_PORT2
        if (i >= STELLA_PINS_PORT1)
          current->port.mask |=
            _BV((i - STELLA_PINS_PORT1) + STELLA_OFFSET_PORT2);
        else
          current->port.mask |= _BV(i + STELLA_OFFSET_PORT1);
#else
        current->port.mask |= _BV(i + STELLA_OFFSET_PORT1);
#endif
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
      // reached the end of the linked list: just append our new entry
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
  current = cal_table->head;
  while (current)
  {
    //debug_printf("%u %s\n", current->value, debug_binary(current->portmask));
    current = current->next;
  }
  debug_printf("Mask1: %s %u\n"
#ifdef STELLA_PINS_PORT2
               "Mask2: %s %u\n"
#endif
		  
               , debug_binary(stella_portmask[0]), stella_portmask[0]
#ifdef STELLA_PINS_PORT2
               , debug_binary(stella_portmask[1]), stella_portmask[1]
#endif
	      );
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
