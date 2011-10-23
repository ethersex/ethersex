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
#ifdef FREQCOUNT_DUTY_SUPPORT
static void freqcount_moving_average(uint32_t freqcount_ticks, uint16_t freqcount_duty);
#else
static void freqcount_moving_average(uint32_t freqcount_ticks);
#endif

// results and moving average data storage for all channels
freqcount_perchannel_data_t freqcount_perchannel_data[FREQCOUNT_CHANNELS];

// takes FREQCOUNT_AVERAGE+2 measurement results
// removes min and max value, calculates average
#ifdef FREQCOUNT_DUTY_SUPPORT
freqcount_average_state_t freqcount_average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty)
#else
freqcount_average_state_t freqcount_average_results(uint32_t freqcount_ticks)
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
        return FC_AVERAGE_IN_PROGRESS;
    }
    else
    {
        uint32_t ticks_result_sum;
#ifdef FREQCOUNT_DUTY_SUPPORT
        uint16_t duty_result_sum;
#endif

        if (freqcount_ticks == 0)
        {
            // we had a timeout, indicate this by setting the results to 0
            ticks_result_sum=0;
#ifdef FREQCOUNT_DUTY_SUPPORT
            duty_result_sum=0;
#endif
        }
        else
        {
            freqcount_ticks_avgsum-=freqcount_ticks_avg_max;
            freqcount_ticks_avgsum-=freqcount_ticks_avg_min;
            ticks_result_sum=freqcount_ticks_avgsum;
            
#ifdef FREQCOUNT_DUTY_SUPPORT
            freqcount_duty_avgsum-=freqcount_duty_avg_max;
            freqcount_duty_avgsum-=freqcount_duty_avg_min;
            duty_result_sum=freqcount_duty_avgsum;
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

#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_moving_average(ticks_result_sum, duty_result_sum);
#else // FREQCOUNT_DUTY_SUPPORT
        freqcount_moving_average(ticks_result_sum);
#endif // FREQCOUNT_DUTY_SUPPORT
        
#else // FREQCOUNT_MOVING_AVERAGE_SUPPORT
        
        freqcount_perchannel_data[freqcount_current_channel].ticks_result_sum=ticks_result_sum;
#ifdef FREQCOUNT_DUTY_SUPPORT
        freqcount_perchannel_data[freqcount_current_channel].duty_result_sum=duty_result_sum;
#endif

        freqcount_new_result_notify();
#endif // FREQCOUNT_MOVING_AVERAGE_SUPPORT
        
        return FC_AVERAGE_DONE;
    }
}

#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT

#if FREQCOUNT_MOVING_AVERAGE_SIZE > 254
#error maximum FREQCOUNT_MOVING_AVERAGE_SIZE of 254 exeeded
#endif

#ifdef FREQCOUNT_DUTY_SUPPORT
static void freqcount_moving_average(uint32_t freqcount_tick_sum, uint16_t freqcount_duty_sum)
#else
static void freqcount_moving_average(uint32_t freqcount_tick_sum)
#endif
{
    // reduce averaging sum to one: makes sure our uint32_t for the sum doesn't overflow later
    if (freqcount_tick_sum < 4294967296ULL/16)
    {
        // correct rounding possible
        freqcount_tick_sum=(freqcount_tick_sum*16)/FREQCOUNT_AVERAGE;
        freqcount_tick_sum+=8;
        freqcount_tick_sum/=16;
    }
    else
        freqcount_tick_sum=freqcount_tick_sum/FREQCOUNT_AVERAGE;
    
#ifdef FREQCOUNT_DUTY_SUPPORT
    uint32_t temp=freqcount_duty_sum*16;
    temp/=FREQCOUNT_AVERAGE;
    temp+=8;
    freqcount_duty_sum=temp/16;
#endif

    freqcount_perchannel_data_t* cur_ch_data=&(freqcount_perchannel_data[freqcount_current_channel]);
    
    if (cur_ch_data->moving_average_pointer==255)
    {
        // initialize freqcount_moving_average_values[] by filling it with one value
        for (cur_ch_data->moving_average_pointer=0; 
             cur_ch_data->moving_average_pointer < FREQCOUNT_MOVING_AVERAGE_SIZE; 
             cur_ch_data->moving_average_pointer++)
        {
            cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].low=(uint16_t)freqcount_tick_sum;

#ifndef FREQCOUNT_NOSLOW_SUPPORT
            cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].high=
                    *(((uint8_t*)&freqcount_tick_sum)+2);
#endif
                
#ifdef FREQCOUNT_DUTY_SUPPORT
            cur_ch_data->moving_average_duty_values[cur_ch_data->moving_average_pointer]=freqcount_duty_sum;
#endif
        }
        
        cur_ch_data->moving_average_pointer=0;
        cur_ch_data->moving_average_sum=freqcount_tick_sum*FREQCOUNT_MOVING_AVERAGE_SIZE;
#ifdef FREQCOUNT_DUTY_SUPPORT
        cur_ch_data->moving_average_duty_sum=freqcount_duty_sum*FREQCOUNT_MOVING_AVERAGE_SIZE;
#endif
    }
    else
    {
        // replace the oldest value by a new one
        
        // increase pointer
        cur_ch_data->moving_average_pointer++;
        if (cur_ch_data->moving_average_pointer == FREQCOUNT_MOVING_AVERAGE_SIZE)
            cur_ch_data->moving_average_pointer=0;
        
#ifdef FREQCOUNT_NOSLOW_SUPPORT
        cur_ch_data->moving_average_sum-=cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].low;
        cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].low=freqcount_tick_sum;
#else
        uint32_t temp=cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].low;
        *(((uint8_t*)&temp)+2)=cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].high;
        cur_ch_data->moving_average_sum-=temp;
        
        cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].low=(uint16_t)freqcount_tick_sum;
        cur_ch_data->moving_average_values[cur_ch_data->moving_average_pointer].high=
                    *(((uint8_t*)&freqcount_tick_sum)+2);
#endif
        cur_ch_data->moving_average_sum+=freqcount_tick_sum;
        
#ifdef FREQCOUNT_DUTY_SUPPORT
        cur_ch_data->moving_average_duty_sum-=cur_ch_data->moving_average_duty_values[cur_ch_data->moving_average_pointer];
        cur_ch_data->moving_average_duty_values[cur_ch_data->moving_average_pointer]=freqcount_duty_sum;
        cur_ch_data->moving_average_duty_sum+=freqcount_duty_sum;
#endif
    }
    
    freqcount_new_result_notify();
}
#endif // FREQCOUNT_MOVING_AVERAGE_SUPPORT

// called whenever we have a new frequency counter result
// if another function or module wants to be notified
// add the call to this function
static void freqcount_new_result_notify(void)
{
#ifdef FREQCOUNT_DEBUGGING
#ifdef FREQCOUNT_DUTY_SUPPORT
    debug_printf("fc chn %u %lu, %lu Hz %u duty\n", freqcount_current_channel, freqcount_get_freq_ticks(freqcount_current_channel),
                 freqcount_get_freq_hz(freqcount_current_channel),freqcount_get_duty(freqcount_current_channel));
#else
    debug_printf("fc chn %u %lu, %lu Hz\n", freqcount_current_channel,
                 freqcount_get_freq_ticks(freqcount_current_channel),freqcount_get_freq_hz(freqcount_current_channel));
#endif
#endif
}

uint32_t freqcount_get_freq_ticks(uint8_t channel)
{
    uint32_t ticks;

#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
    if (freqcount_perchannel_data[channel].moving_average_pointer!=255)
    {
        if (freqcount_perchannel_data[channel].moving_average_sum < 4294967296ULL/16)
        {
            // correct rounding possible
            ticks=(freqcount_perchannel_data[channel].moving_average_sum*16)/FREQCOUNT_MOVING_AVERAGE_SIZE;
            ticks+=8;
            ticks/=16;
        }
        else
            ticks=freqcount_perchannel_data[channel].moving_average_sum/FREQCOUNT_MOVING_AVERAGE_SIZE;
    }
    else
        ticks=0;
#else

    if (freqcount_perchannel_data[channel].ticks_result_sum < 4294967296ULL/16)
    {
        // correct rounding possible
        ticks=(freqcount_perchannel_data[channel].ticks_result_sum*16)/FREQCOUNT_AVERAGE;
        ticks+=8;
        ticks/=16;
    }
    else
        ticks=freqcount_perchannel_data[channel].ticks_result_sum/FREQCOUNT_AVERAGE;
#endif

    return ticks;
}

uint32_t freqcount_get_freq_hz(uint8_t channel)
{
    uint32_t hz;
    
#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
    if (freqcount_perchannel_data[channel].moving_average_pointer!=255)
    {
#if ((FREQCOUNT_CLOCKFREQ*16ULL*FREQCOUNT_MOVING_AVERAGE_SIZE) < 4294967296ULL)
        // numerical correct rounding
        hz=(FREQCOUNT_CLOCKFREQ*FREQCOUNT_MOVING_AVERAGE_SIZE*16)/freqcount_perchannel_data[channel].moving_average_sum;
#else
        hz=(FREQCOUNT_CLOCKFREQ*16)/(freqcount_perchannel_data[channel].moving_average_sum/FREQCOUNT_MOVING_AVERAGE_SIZE);
#endif
        hz+=8;
        hz/=16;
    }
    else
        hz=0;

#else // FREQCOUNT_MOVING_AVERAGE_SUPPORT
        
    if (freqcount_perchannel_data[channel].ticks_result_sum!=0)
    {
#if ((FREQCOUNT_CLOCKFREQ*16ULL*FREQCOUNT_AVERAGE) < 4294967296ULL)
        // numerical correct rounding
        hz=(FREQCOUNT_CLOCKFREQ*FREQCOUNT_AVERAGE*16)/freqcount_perchannel_data[channel].ticks_result_sum;
#else
        hz=(FREQCOUNT_CLOCKFREQ*16)/(freqcount_perchannel_data[channel].ticks_result_sum/FREQCOUNT_AVERAGE);
#endif
        hz+=8;
        hz/=16;
    }
    else
        hz=0;
    
#endif // FREQCOUNT_MOVING_AVERAGE_SUPPORT

    return hz;
}

#ifdef FREQCOUNT_DUTY_SUPPORT
uint8_t freqcount_get_duty(uint8_t channel)
{
    uint32_t duty;

#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
    if (freqcount_perchannel_data[channel].moving_average_pointer!=255)
    {
        duty=freqcount_perchannel_data[channel].moving_average_duty_sum;
        duty*=16;
        duty/=FREQCOUNT_MOVING_AVERAGE_SIZE;
        duty+=8;
        duty/=16;
    }
    else
        duty=0;
#else
    duty=freqcount_perchannel_data[channel].duty_result_sum;
    duty*=16;
    duty/=FREQCOUNT_AVERAGE;
    duty+=8;
    duty/=16;
#endif

    return (uint8_t)duty;
}
#endif // FREQCOUNT_DUTY_SUPPORT


/*
  -- Ethersex META --
  header(services/freqcount/freqcount.h)
*/
