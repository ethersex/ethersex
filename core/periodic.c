/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) by David Gräff <david.graeff@web.de>
 * Copyright (c) 2014 by Michael Brakemeier <michael@brakemeier.de>
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

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

#include "config.h"

#include "core/periodic.h"
#include "core/debug.h"
#ifdef FREQCOUNT_SUPPORT
#include "services/freqcount/freqcount.h"
#endif
#ifdef CLOCK_PERIODIC_SUPPORT
#include "services/clock/clock.h"
#endif

#ifdef DEBUG_PERIODIC
#define PERIODICDEBUG(a...)   debug_printf("periodic: " a)
volatile uint16_t periodic_milliticks_min;
volatile uint16_t periodic_milliticks_max;
volatile uint16_t periodic_milliticks_last;
volatile uint16_t periodic_milliticks_miss;
#else
#define PERIODICDEBUG(a...)
#endif /* DEBUG_PERIODIC */

#ifdef BOOTLOADER_SUPPORT
uint16_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

/* signal a new timer() tick to meta */
extern volatile uint8_t newtick;

/* millitick counter */
volatile uint16_t periodic_milliticks;

void
periodic_init(void)
{
  // Well, WriteAccess to 16bit timer registers has to be protected
  // but as TimerInts are expected to be disabled here,
  // hopefully nobody else will touch timer registers right now,
  // and we can save some code bytes in not using ATOMIC_BLOCK!

  CLOCK_SET_PRESCALER;

#ifdef FREQCOUNT_SUPPORT
  /* init timer1 to run with full cpu frequency, normal mode,
   * compare and overflow int active */
  TC1_PRESCALER_1;
  freqcount_init();
  TC1_INT_COMPARE_ON;
  TC1_INT_OVERFLOW_ON;
#else
/*
 * !! *DO NOT TOUCH* the Timer/Counter registers used by the periodic
 * !! framework except through the interfaces provided!
 */
  periodic_milliticks = 0;

#ifdef DEBUG_PERIODIC
  periodic_milliticks_min = CONF_CLOCKS_PER_SEC;
  periodic_milliticks_max = 0;
  periodic_milliticks_last = 0;
  periodic_milliticks_miss = 0;
#endif

/* Configure 16 bit Timer/Counter for normal mode, counting up from
 * (MAX_OVERFLOW + 1 - (F_CPU / CLOCK_PRESCALER / HZ)) to MAX_OVERFLOW.
 * Use TOV interrupts to generate 50Hz / 20ms intervals for timer() calls
 * from meta. Use OCR interrupts to generate periodic milliticks.
 */
  PERIODIC_MODE_OFF;
  PERIODIC_COUNTER_CURRENT = PERIODIC_ZERO;
  // the last compare match matches the overflow condition,
  // subtract one here to avoid losing the related tick
  PERIODIC_COUNTER_COMPARE = PERIODIC_ZERO + CLOCK_MILLITICKS - 1;
  PERIODIC_INT_COMPARE_ON;
  PERIODIC_INT_OVERFLOW_ON;
#endif /* FREQCOUNT_SUPPORT */
}

/**
 * Timer/Counter compare ISR with milliticks support is auto-generated!
 * See core/periodic_milliticks.c for
 *
 * ISR(PERIODIC_VECTOR_COMPARE)
 */

/**
 * Timer/Counter overflow ISR with milliticks support.
 *
 * This ISR is called every 20ms / 50 times per second.
 */
ISR(PERIODIC_VECTOR_OVERFLOW)
{
  // reset timer
  PERIODIC_COUNTER_CURRENT = PERIODIC_ZERO;
  // the last compare match matches the overflow condition,
  // subtract one here to avoid losing the related tick
  PERIODIC_COUNTER_COMPARE = PERIODIC_ZERO + CLOCK_MILLITICKS - 1;

  // provide tick for metas timer() calls
  newtick = 1;

#ifdef CLOCK_PERIODIC_SUPPORT
  /* tick the clock */
  clock_tick();
#endif

#ifdef DEBUG_PERIODIC_WAVEFORMS_SUPPORT
  PIN_TOGGLE(PERIODIC_WAVE25HZ_OUT);
#endif

#ifdef DEBUG_PERIODIC
  if (periodic_milliticks > periodic_milliticks_max)
    periodic_milliticks_max = periodic_milliticks;

  // CLOCKS_PER_SEC/HZ ticks since last TOV
  if (periodic_milliticks < (periodic_milliticks_last + (CONF_CLOCKS_PER_SEC / HZ)))
    periodic_milliticks_miss += (periodic_milliticks_last + (CONF_CLOCKS_PER_SEC / HZ) - periodic_milliticks);
#endif

  // should be an exact match here
  if (periodic_milliticks >= CONF_CLOCKS_PER_SEC)
    periodic_milliticks -= CONF_CLOCKS_PER_SEC;

#ifdef DEBUG_PERIODIC
  if (periodic_milliticks < periodic_milliticks_min)
    periodic_milliticks_min = periodic_milliticks;

  periodic_milliticks_last = periodic_milliticks;
#endif
}

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
