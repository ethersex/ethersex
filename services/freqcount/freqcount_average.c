/*
* frequency counter
* data averaging, result handling
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

static void freqcount_new_result_notify(void);

uint32_t freqcount_ticks_result=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
uint8_t freqcount_duty_result=0;
#endif

// takes FREQCOUNT_AVERAGE+2 measurement results
// removes min and max value, calculates average
#ifdef FREQCOUNT_DUTY_SUPPORT
void freqcount_average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty)
#else
void freqcount_average_results(uint32_t freqcount_ticks)
#endif
{
    static uint32_t freqcount_ticks_avgsum=0;
    static uint32_t freqcount_ticks_avg_max=0;
    static uint32_t freqcount_ticks_avg_min=0xFFFFFFFF;
    
#ifdef FREQCOUNT_DUTY_SUPPORT
    static uint16_t freqcount_duty_avgsum=0;
    static uint16_t freqcount_duty_avg_max=0;
    static uint16_t freqcount_duty_avg_min=0xFFFF;
#endif
    
    static uint8_t freqcount_avg_cnt=0;

    // collect FREQCOUNT_AVERAGE+2 values: min and max are cut off
    if (freqcount_avg_cnt < FREQCOUNT_AVERAGE+2 && freqcount_ticks != 0)
    {
        freqcount_ticks_avgsum+=freqcount_ticks;
        if (freqcount_ticks > freqcount_ticks_avg_max)
            freqcount_ticks_avg_max=freqcount_ticks;
        else if (freqcount_ticks < freqcount_ticks_avg_min)
            freqcount_ticks_avg_min=freqcount_ticks;

#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_duty_avgsum+=freqcount_duty;
        if (freqcount_duty > freqcount_duty_avg_max)
            freqcount_duty_avg_max=freqcount_duty;
        else if (freqcount_duty < freqcount_duty_avg_min)
            freqcount_duty_avg_min=freqcount_duty;
#endif

        freqcount_avg_cnt++;
    }
    else
    {
        if (freqcount_ticks == 0)
        {
            // we had a timeout, indicate this by setting the results to 0
            freqcount_ticks_result=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
            freqcount_duty_result=0;
#endif
        }
        else
        {
            freqcount_ticks_avgsum-=freqcount_ticks_avg_max;
            freqcount_ticks_avgsum-=freqcount_ticks_avg_min;
            freqcount_ticks_result=freqcount_ticks_avgsum/FREQCOUNT_AVERAGE;
            
#ifdef FREQCOUNT_DUTY_SUPPORT
            freqcount_duty_avgsum-=freqcount_duty_avg_max;
            freqcount_duty_avgsum-=freqcount_duty_avg_min;
            freqcount_duty_result=freqcount_duty_avgsum/FREQCOUNT_AVERAGE;
#endif
        }

        // reset averaging
        freqcount_ticks_avgsum=0;
        freqcount_ticks_avg_max=0;
        freqcount_ticks_avg_min=0xFFFFFFFF;
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_duty_avgsum=0;
        freqcount_duty_avg_max=0;
        freqcount_duty_avg_min=0xFFFF;
#endif
        
        freqcount_avg_cnt=0;

        freqcount_new_result_notify();
    }
}

// called whenever we have a new frequency counter result
// if another function or module wants to be notified
// add the call to this function
static void freqcount_new_result_notify(void)
{
#ifdef FREQCOUNT_DEBUGGING
#ifdef FREQCOUNT_DUTY_SUPPORT
    debug_printf("fc ticks %lu, %lu Hz %u duty\n", freqcount_ticks_result,
                 freqcount_get_freq_hz(),freqcount_duty_result);
#else
    debug_printf("fc ticks %lu, %lu Hz\n", freqcount_ticks_result,freqcount_get_freq_hz());
#endif
#endif
}

uint32_t freqcount_get_freq_hz(void)
{
    uint32_t hz;
    
    if (freqcount_ticks_result==0)
        hz=0;
    else
    {
        // numerical correct rounding
        hz=(FREQCOUNT_CLOCKFREQ*10)/freqcount_ticks_result;
        hz+=5;
        hz/=10;
    }

    return hz;
}

/*
  -- Ethersex META --
  header(services/freqcount/freqcount.h)
*/
