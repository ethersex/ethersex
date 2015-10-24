/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) by David Gräff <david.graeff@web.de>
 * Copyright (c) 2013-2015 by Michael Brakemeier <michael@brakemeier.de>
 * Copyright (c) 2015 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include <avr/interrupt.h>

#include <util/atomic.h>

#include "config.h"

#include "core/periodic.h"
#ifdef FREQCOUNT_SUPPORT
#include "services/freqcount/freqcount.h"
#endif

#ifdef DEBUG_PERIODIC
volatile uint16_t pdebug_milliticks;
volatile uint16_t pdebug_milliticks_max;
volatile uint16_t pdebug_milliticks_last;
volatile uint16_t pdebug_milliticks_miss;
#endif /* DEBUG_PERIODIC */

#ifdef BOOTLOADER_SUPPORT
uint16_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

/* periodic milliticks counter */
volatile uint32_t periodic_mticks_count;

void
periodic_init(void)
{
  /*
   * !! *DO NOT TOUCH* the Timer/Counter registers used by the periodic
   * !! framework except through the interfaces provided!
   */
  PERIODIC_SET_PRESCALER;

#ifdef FREQCOUNT_SUPPORT
  /* init timer1 to run with full cpu frequency, normal mode,
   * compare and overflow int active */
  TC1_PRESCALER_1;
  freqcount_init();
  TC1_INT_COMPARE_ON;
  TC1_INT_OVERFLOW_ON;
#else

  PERIODIC_MODE_PWMFAST_OCR;
  PERIODIC_COUNTER_CURRENT = 0;
  PERIODIC_COUNTER_COMPARE = PERIODIC_TOP;
  PERIODIC_INT_OVERFLOW_ON;

#if defined(DEBUG_PERIODIC) && defined(DEBUG_PERIODIC_OC1A_SUPPORT)
  TC1_OUTPUT_COMPARE_TOGGLE;
#endif
#endif /* FREQCOUNT_SUPPORT */
}

/**
 * Timer/Counter overflow ISR with milliticks support is auto-generated!
 * See core/periodic_milliticks.c for
 *
 * ISR(PERIODIC_VECTOR_OVERFLOW)
 */

#ifdef FREQCOUNT_SUPPORT
void
timer_expired(void)
{
  newtick = 1;

 if (++milliticks >= HZ)
    milliticks -= HZ;
}
#endif

#ifdef PERIODIC_ADJUST_SUPPORT
uint16_t
periodic_adjust_set_offset(int16_t offset)
{
  /* check under- and overflow and limit offset to 25% */
  if ((abs(offset) < (PERIODIC_TOP / 4)) &&
      ((int32_t) offset < (int32_t) (MAX_OVERFLOW - PERIODIC_TOP)))
  {
    /* OK, set new top.
     * Timer Fast PWM mode uses double buffering of OCR value,
     * no need for special timing to set new value.
     */
    PERIODICDEBUG("Adjust by %d, new TOP %u\n", offset,
                  (PERIODIC_TOP + offset));

    PERIODIC_COUNTER_COMPARE = PERIODIC_TOP + offset;

    return PERIODIC_COUNTER_COMPARE;
  }

  PERIODICDEBUG("Adjust not allowed, offset was %d\n", offset);

  return 0;
}
#endif

#ifdef PERIODIC_TIMER_API_SUPPORT
void
periodic_milliticks(periodic_timestamp_t * now)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    now->fragments = PERIODIC_COUNTER_CURRENT;
    now->ticks = periodic_mticks_count;
  }
}

#ifdef PERIODIC_ADJUST_SUPPORT
#define FRAGMENTS_PER_TICK (PERIODIC_COUNTER_COMPARE + 1)
#else
#define FRAGMENTS_PER_TICK (PERIODIC_TOP + 1)
#endif

#include "core/debug.h"

uint32_t
periodic_micros_elapsed(periodic_timestamp_t * last)
{
  periodic_timestamp_t now;
  periodic_milliticks(&now);
  return periodic_micros_diff(last, &now);
}

uint32_t
periodic_micros_diff(periodic_timestamp_t * t1, periodic_timestamp_t * t2)
{
  uint32_t ticks = t2->ticks;
  uint32_t fragments = t2->fragments;

#ifdef DEBUG_PERIODIC
  if (t1->ticks == ticks && t1->fragments > fragments)
    debug_printf("periodic_micros_diff: l=%lu,%u n=%lu,%lu\n",
                 t1->ticks, t1->fragments, ticks, fragments);
#endif

  /* calculate elapsed time in microseconds */
  if (t1->ticks <= ticks)
  {
    ticks = ticks - t1->ticks;
  }
  else
  {
    ticks = UINT32_MAX - t1->ticks + ticks;
  }

  if (t1->fragments <= fragments)
  {
    fragments = fragments - t1->fragments;
  }
  else
  {
    fragments = FRAGMENTS_PER_TICK - t1->fragments + fragments;
    if (ticks != 0)
      ticks -= 1;
  }

  return (uint32_t) (((1000000ULL * (uint64_t) ticks) / CONF_MTICKS_PER_SEC) +
                     ((1000000ULL * (uint64_t) fragments) /
                      (CONF_MTICKS_PER_SEC * FRAGMENTS_PER_TICK)));
}

#endif

#ifdef FREQCOUNT_SUPPORT
void
timer_expired(void)
{
  newtick = 1;
  if (++milliticks >= HZ)
    milliticks -= HZ;
}
#endif

/*
 -- Ethersex META --
 header(core/periodic.h)
 init(periodic_init)
 */
