/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) by David Gräff <david.graeff@web.de>
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

#include "config.h"
#include "core/periodic.h"
#include "core/debug.h"
#ifdef FREQCOUNT_SUPPORT
#include "services/freqcount/freqcount.h"
#endif

#ifdef BOOTLOADER_SUPPORT
uint8_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

extern volatile uint8_t newtick;
uint8_t milliticks;

void
periodic_init(void)
{
  // Well, WriteAccess to 16bit timer registers has to be protected
  // but as Timer1Ints are expected to be disabled here,
  // hopefully nobody else will touch timer1 registers right now, 
  // and we can save some code bytes in not using ATOMIC_BLOCK!

  CLOCK_SET_PRESCALER;
#ifdef CLOCK_CPU_SUPPORT
  /* init timer1 to expire after ~20ms, with Normal */
  TC1_MODE_OFF;
  TC1_COUNTER_CURRENT = 65536 - CLOCK_SECONDS;
  TC1_COUNTER_COMPARE = 65536 - CLOCK_SECONDS + CLOCK_TICKS;
  TC1_INT_COMPARE_ON;
  TC1_INT_OVERFLOW_ON;
#else
#ifdef FREQCOUNT_SUPPORT
  /* init timer1 to run with full cpu frequency, normal mode, 
   * compare and overflow int active */
  TC1_PRESCALER_1;
  freqcount_init();
  TC1_INT_COMPARE_ON;
  TC1_INT_OVERFLOW_ON;
#else
  /* init timer1 to expire after ~20ms, with CTC enabled */
  TC1_MODE_CTC;
  TC1_COUNTER_COMPARE = (F_CPU / CLOCK_PRESCALER / HZ) - 1;
  TC1_INT_COMPARE_ON;
  NTPADJDEBUG("configured OCR1A to %d\n", TC1_COUNTER_COMPARE);
#endif
#endif
}

#ifdef FREQCOUNT_SUPPORT
void
timer_expired(void)
#else
ISR(TC1_VECTOR_COMPARE)
#endif
{
#ifdef CLOCK_CPU_SUPPORT
  TC1_COUNTER_COMPARE += CLOCK_TICKS;
#endif
  newtick = 1;
  if (++milliticks >= HZ)
    milliticks -= HZ;
}

/*
 -- Ethersex META --
 header(core/periodic.h)
 init(periodic_init)
 */
