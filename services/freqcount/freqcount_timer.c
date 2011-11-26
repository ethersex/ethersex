/*
* frequency counter
* timer interrupt handling
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
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
#include "core/bit-macros.h"
#include "core/debug.h"
#include "core/periodic.h"

#include "freqcount.h"
#include "freqcount_internal.h"

#ifndef FREQCOUNT_NOSLOW_SUPPORT
// increased every 65536 clock cycles
// is a 24 bit clock together with TCNT1
volatile uint8_t timer_overflows;
#endif

// full overflows needed to next clock tick
volatile uint8_t overflows_to_clocktick;

#define FULL_OVERFLOWS ((FREQCOUNT_CLOCKFREQ/MAX_OVERFLOW/HZ)+1)
#define REMAINING_TICKS ((uint16_t)((FREQCOUNT_CLOCKFREQ%(MAX_OVERFLOW*HZ))/HZ))

#if (FULL_OVERFLOWS < 2)
#error cpu frequency too low for frequency counter 
#endif

void freqcount_init (void)
{
    TC1_COUNTER_CURRENT = 0;
    TC1_COUNTER_COMPARE = REMAINING_TICKS;
    overflows_to_clocktick=FULL_OVERFLOWS;

#ifndef FREQCOUNT_NOSLOW_SUPPORT
    timer_overflows=0;
#endif
}

// timer overflow
inline void timer_overflow()
{
#ifndef FREQCOUNT_NOSLOW_SUPPORT
    timer_overflows++;
    
    // did we have an overflow of timer_overflows?
    if (timer_overflows==0)
#endif   // this endif is in between if and { intentionally, no bug!
    {
        // we had an overflow
        // either of timer_overflows or of TCNT1
        freqcount_state++;
    }
        
    overflows_to_clocktick--;
}

ISR (TC1_VECTOR_OVERFLOW)
{
    timer_overflow();
}

// timer compare
ISR(TC1_VECTOR_COMPARE)
{
    // did the overflow and compare happen at once?
    // make sure that the overflow vector is executed first
    // ignore cases where compare triggered first but could
    // not be handled yet. Limit at half the possible range for this.
    if (TC1_INT_OVERFLOW_TST && TC1_COUNTER_COMPARE < 32768)
    {
        timer_overflow();
        // disable the int flag as we already have handled it
        TC1_INT_OVERFLOW_CLR
    }
    
    // we only have to look for incomplete timer overflow cycles
    if (overflows_to_clocktick==0)
    {
        overflows_to_clocktick=FULL_OVERFLOWS;
        TC1_COUNTER_COMPARE=REMAINING_TICKS-MAX_OVERFLOW+OCR1A;
        if (TC1_COUNTER_COMPARE > REMAINING_TICKS)
            overflows_to_clocktick--;

        // call the regular ISR for timer expired condition (every 20ms)
        timer_expired();
    }
}

/*
  -- Ethersex META --
  header(services/freqcount/freqcount.h)
*/
