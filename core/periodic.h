/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _PERIODIC_H
#define _PERIODIC_H

#include <stdint.h>

#include <avr/io.h>

#include "config.h"

/* 16 Bit Timer/Counter settings for periodic module.
 *
 * Timer/Counter1/3 will be configured for Fast PWM mode, counting up from
 * 0 to TOP. TOP is set via the Output Compare Register (OCRnA). Overflow
 * interrupts (TOV) will be used to generate periodic milliticks.
 * Using Fast PWM and OCRnA ensures double buffered access when writing
 * updated TOP values.
 *
 * Calculate periodic timer constants from CONF_MTICKS_PER_SEC:
 * Aim at exactness of the periodic milliticks frequency. Calculate its
 * period time (equal to TOP) first and then deduce all other timings
 * from this one.
 *
 * Formula for Fast PWM acc. to AVR datasheet:
 * CONF_MTICKS_PER_SEC = F_CPU / (PERIODIC_PRESCALER * (1 + PERIODIC_TOP))
 * => PERIODIC_TOP = (F_CPU / (PERIODIC_PRESCALER * CONF_MTICKS_PER_SEC)) - 1
 */

/* 16 bit timer/counter 1/3 upper limit */
#define MAX_OVERFLOW            65535UL
/* e6 basic timer frequency 50 Hz / period time 20 ms*/
#define HZ                      50

/* Calculate required prescaler value, aim for highest timer/counter
 * resolution, i.e. lowest possible prescaler.
 *
 * PERIODIC_PRESCALER = F_CPU / (CONF_MTICKS_PER_SEC * (1 + MAX_OVERFLOW))
 */
#if F_CPU > (CONF_MTICKS_PER_SEC * (1 + MAX_OVERFLOW))
/* Set prescaler to divide F_CPU by 8 */
#define PERIODIC_PRESCALER      8UL
#define PERIODIC_SET_PRESCALER  PERIODIC_PRESCALER_8
#else
/* Use F_CPU without prescaler to tick the timer */
#define PERIODIC_PRESCALER      1UL
#define PERIODIC_SET_PRESCALER  PERIODIC_PRESCALER_1
#endif

/* Periodic milliticks per second / periodic_milliticks_isr frequency.
 * CONF_MTICKS_PER_SEC          defined via config.in/autoconf.h
 */
/* Timer ticks needed for one periodic millitick. Fast PWM TOP value. */
#define PERIODIC_TOP            ((F_CPU / (PERIODIC_PRESCALER * CONF_MTICKS_PER_SEC)) - 1)
/* Timer ticks needed for ~one second. */
#define PERIODIC_TTICKS_PER_SEC (PERIODIC_TOP * CONF_MTICKS_PER_SEC)

/* Macro to convert milliseconds to periodic milliticks */
#define PERIODIC_MS2MTICKS(msecs) ((msecs) * CONF_MTICKS_PER_SEC / 1000)

/* Periodic milliticks usability check -
 * there should be at least ~1000 CPU-Cycles per periodic millitick
 */
#if (F_CPU / CONF_MTICKS_PER_SEC) < 1000
#warning *** You should either increase F_CPU or decrease the tick
#warning *** rate to make periodic milliticks work as expected!!
#endif

#ifdef DEBUG_PERIODIC
#include "core/debug.h"

#define PERIODICDEBUG(a...)      debug_printf("periodic: " a)
extern volatile uint16_t pdebug_milliticks;
extern volatile uint16_t pdebug_milliticks_max;
extern volatile uint16_t pdebug_milliticks_last;
extern volatile uint16_t pdebug_milliticks_miss;
#else
#define PERIODICDEBUG(a...)
#endif /* DEBUG_PERIODIC */

/* periodic milliticks counter */
extern volatile uint32_t periodic_mticks_count;

/* Initialize hardware timer. */
void periodic_init(void);

#ifdef PERIODIC_ADJUST_SUPPORT
/**
 * Adjust the periodic timer TOP value.
 *
 * @param offset +/-25% offset from PERIODIC_TOP in timer ticks,
 *      0 means reset to PERIODIC_TOP.
 * @return the new TOP value or 0 if PERIODIC_TOP +/-offset would
 *      exceed the allowable range.
 */
uint16_t periodic_adjust_set_offset(int16_t offset);
#endif

#ifdef PERIODIC_TIMER_API_SUPPORT

typedef struct
{
  uint32_t ticks;
  uint16_t fragments;
} periodic_timestamp_t;

/**
 * Returns the number of periodic milliticks and fragments since
 * the e6 periodic framework has been initialized.
 */
void periodic_milliticks(periodic_timestamp_t * now);

/**
 * Returns the number of microseconds that have elapsed between
 * last and now.
 */
uint32_t periodic_micros_elapsed(periodic_timestamp_t * last);

/**
 * Returns the number of microseconds between timestamps t1 and t2.
 */
uint32_t periodic_micros_diff(periodic_timestamp_t * t1,
                              periodic_timestamp_t * t2);

/**
 * Returns the number of milliseconds that have elapsed between
 * last and now.
 */
#define periodic_millis_elapsed(t) (periodic_micros_elapsed(t)/1000)

/**
 * Returns the number of milliseconds thbetween timestamps t1 and t2.
 */
#define periodic_millis_diff(t1,t2) (periodic_micros_diff(t1,t2)/1000)

#endif

#ifdef FREQCOUNT_SUPPORT
void timer_expired(void);
#endif

#endif /* _PERIODIC_H */
