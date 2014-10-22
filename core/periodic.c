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

  // TODO check for needed stuff from clock.c ISR
  //#ifdef DCF77_SUPPORT
  //  dcf77_tick();
  //#endif
  //
  //#ifdef CLOCK_CPU_SUPPORT
  //  milliticks = 0;
  //
  //  TC1_COUNTER_CURRENT = 65536 - CLOCK_SECONDS;
  //  TC1_COUNTER_COMPARE = 65536 - CLOCK_SECONDS + CLOCK_TICKS;
  //#endif
  //  milliticks = 0;
  //
  //  TC1_COUNTER_CURRENT = 65536 - CLOCK_SECONDS;
  //  TC1_COUNTER_COMPARE = 65536 - CLOCK_SECONDS + CLOCK_TICKS;
  //#endif
  //
  //#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
  //  if (!sync_timestamp || sync_timestamp == clock_timestamp)
  //#endif
  //  {
  //    clock_timestamp++;
  //#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT) || defined(CONTROL6_SUPPORT)
  //    uptime_timestamp++;
  //#endif
  //  }
  //
  //  if (sync_timestamp)
  //    sync_timestamp++;
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

uint32_t
periodic_micros_elapsed(periodic_timestamp_t * last)
{
  uint32_t ticks;
  uint32_t fragments;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    fragments = PERIODIC_COUNTER_CURRENT;
    ticks = periodic_mticks_count;
  }

  /* calculate elapsed time in microseconds */
  if (last->ticks <= ticks)
  {
    ticks = ticks - last->ticks;
  }
  else
  {
    ticks = UINT32_MAX - last->ticks + ticks;
  }

  if (last->fragments <= fragments)
  {
    fragments = fragments - last->fragments;
  }
  else
  {
    fragments = FRAGMENTS_PER_TICK - last->fragments + fragments;
    ticks -= 1;
  }

  return (uint32_t) (((1000000ULL * (uint64_t) ticks) / CONF_MTICKS_PER_SEC) +
                     ((1000000ULL * (uint64_t) fragments) /
                      (CONF_MTICKS_PER_SEC * FRAGMENTS_PER_TICK)));
}

uint32_t
periodic_millis_elapsed(periodic_timestamp_t * last)
{
  return periodic_micros_elapsed(last) / 1000;
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
