/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _PERIODIC_H
#define _PERIODIC_H

#include <stdint.h>

#include <avr/io.h>

#include "config.h"

#define MAX_OVERFLOW          65535UL   /* 16 bit timer/counter 1/3 max */
#define HZ                    50        /* 20ms */

/* periodic milliticks usability check -
 * there should be at least ~2000 CPU-Cycles per Tick
 */
#if (F_CPU / CLOCKS_PER_SEC) < 2000
#warning *** You should either increase F_CPU or decrease
#warning *** the tick rate to make periodic milliticks work as expected!!
#endif

/* Determine 16 Bit Timer/Counter settings.
 * TC1/3 will be configured for normal mode, counting up from
 * (MAX_OVERFLOW + 1 - (F_CPU / CLOCK_PRESCALER / HZ)) to MAX_OVERFLOW.
 * TOV interrupts will be used to generate 50Hz / 20ms intervals for timer()
 * calls from meta. OCR interrupts will be used to generate milliticks.
 * CPU_CLOCK_SUPPORT uses a simple counter on top of TOV.
 */
#if F_CPU >= (HZ * (MAX_OVERFLOW + 1))  /* break frequency 3.2768MHz */
/* Set prescaler to divide F_CPU by 8 */
#define CLOCK_PRESCALER         8UL
#define CLOCK_SET_PRESCALER     PERIODIC_PRESCALER_8
/* Huh? F_CPU > 26.214400MHz. FIXME for xmega support */
#if (F_CPU/CLOCK_PRESCALER) >= (HZ * (MAX_OVERFLOW + 1))
#error Validate F_CPU! Could not determine millitick timer settings.
#endif
#else
/* F_CPU < 3.2768MHz, no prescaler required */
#define CLOCK_PRESCALER         1UL
#define CLOCK_SET_PRESCALER     PERIODIC_PRESCALER_1
#endif

/* timer ticks needed for one millitick */
#define CLOCK_MILLITICKS        (F_CPU/CLOCK_PRESCALER/CLOCKS_PER_SEC)
/* timer ticks needed for one 20ms clock tick */
#define CLOCK_TICKS             (F_CPU/CLOCK_PRESCALER/HZ)
/* timer ticks needed for one second */
#define CLOCK_SECONDS           (F_CPU/CLOCK_PRESCALER)
/* Timer starts at "zero" */
#define PERIODIC_ZERO           ((MAX_OVERFLOW + 1) - CLOCK_TICKS)

/* millitick counter */
extern volatile uint16_t milliticks;

/* initialize hardware timer */
void periodic_init(void);

/* reset Timer/Counter to zero, starting a new cycle */
void periodic_reset_tick(void);

//#ifdef FREQCOUNT_SUPPORT
//void timer_expired(void);
//#endif

#endif /* _PERIODIC_H */
