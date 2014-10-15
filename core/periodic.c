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
//#ifdef FREQCOUNT_SUPPORT
//#include "services/freqcount/freqcount.h"
//#endif

#ifdef DEBUG_PERIODIC
#define PERIODICDEBUG(a...)   debug_printf("periodic: " a)
volatile uint16_t milliticks_min;
volatile uint16_t milliticks_max;
volatile uint16_t milliticks_last;
volatile uint16_t milliticks_miss;
#else
#define PERIODICDEBUG(a...)
#endif /* DEBUG_PERIODIC */

#ifdef BOOTLOADER_SUPPORT
uint16_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

extern volatile uint8_t newtick;
volatile uint16_t milliticks;

void
periodic_init(void)
{
  // Well, WriteAccess to 16bit timer registers has to be protected
  // but as TimerInts are expected to be disabled here,
  // hopefully nobody else will touch timer registers right now,
  // and we can save some code bytes in not using ATOMIC_BLOCK!

  CLOCK_SET_PRESCALER;

//#ifdef CLOCK_CPU_SUPPORT
//  /* init timer1 to expire after ~20ms, with Normal */
//  TC1_MODE_OFF;
//  TC1_COUNTER_CURRENT = 65536 - CLOCK_SECONDS;
//  TC1_COUNTER_COMPARE = 65536 - CLOCK_SECONDS + CLOCK_TICKS;
//  TC1_INT_COMPARE_ON;
//  TC1_INT_OVERFLOW_ON;
//#else
//#ifdef FREQCOUNT_SUPPORT
//  /* init timer1 to run with full cpu frequency, normal mode,
//   * compare and overflow int active */
//  TC1_PRESCALER_1;
//  freqcount_init();
//  TC1_INT_COMPARE_ON;
//  TC1_INT_OVERFLOW_ON;
//#else
/*
 * !! *DO NOT TOUCH* the Timer/Counter registers used by the periodic
 * !! framework except through the interfaces provided!
 */
#ifdef DEBUG_PERIODIC
  milliticks_min = CLOCKS_PER_SEC;
  milliticks_max = 0;
  milliticks_last = 0;
  milliticks_miss = 0;
#endif

/* Configure 16 bit Timer/Counter for normal mode, counting up from
 * (MAX_OVERFLOW + 1 - (F_CPU / CLOCK_PRESCALER / HZ)) to MAX_OVERFLOW.
 * Use TOV interrupts to generate 50Hz / 20ms intervals for timer() calls
 * from meta. Use OCR interrupts to generate periodic milliticks.
 */
  milliticks = 0;

  PERIODIC_MODE_OFF;
  PERIODIC_COUNTER_CURRENT = PERIODIC_ZERO;
  // the last compare match matches the overflow condition,
  // subtract one here to avoid losing the related tick
  PERIODIC_COUNTER_COMPARE = PERIODIC_ZERO + CLOCK_MILLITICKS - 1;
  PERIODIC_INT_COMPARE_ON;
  PERIODIC_INT_OVERFLOW_ON;
//#endif /* FREQCOUNT_SUPPORT */
//#endif /* CLOCK_CPU_SUPPORT */
}

/**
 * Timer/Counter compare ISR with milliticks support is auto-generated!
 * See core/milliticks.c for
 *
 * ISR(PERIODIC_VECTOR_COMPARE)
 */

/**
 * Timer/Counter overflow ISR with milliticks support.
 */
ISR(PERIODIC_VECTOR_OVERFLOW)
{
  // reset timer
  PERIODIC_COUNTER_CURRENT = PERIODIC_ZERO;
  // the last compare match matches the overflow condition,
  // subtract one here to avoid losing the related tick
  PERIODIC_COUNTER_COMPARE = PERIODIC_ZERO + CLOCK_MILLITICKS - 1;

#ifdef DEBUG_PERIODIC_WAVEFORMS_SUPPORT
  PIN_TOGGLE(PERIODIC_WAVE25HZ_OUT);
#endif

#ifdef DEBUG_PERIODIC
  if (milliticks > milliticks_max)
    milliticks_max = milliticks;

  // CLOCKS_PER_SEC/HZ ticks since last TOV
  if (milliticks < (milliticks_last + (CLOCKS_PER_SEC / HZ)))
    milliticks_miss += (milliticks_last + (CLOCKS_PER_SEC / HZ) - milliticks);
#endif

  // should be an exact match here
  if (milliticks >= CLOCKS_PER_SEC)
    milliticks -= CLOCKS_PER_SEC;

#ifdef DEBUG_PERIODIC
  if (milliticks < milliticks_min)
    milliticks_min = milliticks;

  milliticks_last = milliticks;
#endif

  // provide tick for metas timer() calls
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

void
periodic_reset_tick(void)
{
/* TODO support other configurations and rewrite any and all
 * direct TC register accesses from other modules.
 */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // reset timer
    PERIODIC_COUNTER_CURRENT = PERIODIC_ZERO;
    // the last compare match matches the overflow condition,
    // subtract one here to avoid losing the related tick
    PERIODIC_COUNTER_COMPARE = PERIODIC_ZERO + CLOCK_MILLITICKS - 1;
    // TODO reset milliticks, newtick, dcf_tick...??
  }
}

//#ifdef FREQCOUNT_SUPPORT
//void
//timer_expired(void)
//#else
//ISR(TC1_VECTOR_COMPARE)
//#endif
//{
//#ifdef CLOCK_CPU_SUPPORT
//  TC1_COUNTER_COMPARE += CLOCK_TICKS;
//#endif
//  newtick = 1;
//  if (++milliticks >= HZ)
//    milliticks -= HZ;
//}

/*
 -- Ethersex META --
 header(core/periodic.h)
 init(periodic_init)
 */
