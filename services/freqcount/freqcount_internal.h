/*
*
* internal functions of the frequency counter
* not intended for use from outside the frequency counter
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

#ifndef FREQCOUNT_INTERNAL_H
#define FREQCOUNT_INTERNAL_H

struct tick24bit
{
#ifndef FREQCOUNT_NOSLOW_SUPPORT
    uint8_t high;
#endif
    uint16_t low;
};
typedef struct tick24bit tick24bit_t;

struct freqcount_perchannel_data
{
    uint8_t enabled;

#ifdef FREQCOUNT_MOVING_AVERAGE_SUPPORT
    uint32_t moving_average_sum;
    tick24bit_t moving_average_values[FREQCOUNT_MOVING_AVERAGE_SIZE];

#ifdef FREQCOUNT_DUTY_SUPPORT
    uint16_t moving_average_duty_sum;
    uint8_t moving_average_duty_values[FREQCOUNT_MOVING_AVERAGE_SIZE];
#endif

    // pointer to the last value written within freqcount_moving_average_values[]
    // magic value 255: we don't have written anything at all, 
    // freqcount_moving_average_values has to be initialized
    uint8_t moving_average_pointer;

#else // FREQCOUNT_MOVING_AVERAGE_SUPPORT
    
    // stored per channel if we don't have moving averages
    uint32_t ticks_result_sum;
#ifdef FREQCOUNT_DUTY_SUPPORT
    uint16_t duty_result_sum;
#endif

#endif // FREQCOUNT_MOVING_AVERAGE_SUPPORT
};
typedef struct freqcount_perchannel_data freqcount_perchannel_data_t;

extern freqcount_perchannel_data_t freqcount_perchannel_data[FREQCOUNT_CHANNELS];

extern uint8_t freqcount_current_channel_store;

#if FREQCOUNT_CHANNELS > 1
#define freqcount_current_channel freqcount_current_channel_store
#else
#define freqcount_current_channel 0
#endif


// freqcount_state is used for two things:
// 1. communicate the current state of measurement
//    between ISR and control logic
// 2. count the overflows of the tick timer
//    (used to detect a frequency lower than we can measure)
//    freqcount_state is increased by one every overflow.
//    check_measure_timeout() bails out if it sees *_OVERFLOW2.
//    *_OVERFLOW3 and 4 are safety margin against times with
//    very high interrupt load.
// this dual use keeps the ISR hot path short and saves ram
enum freqcount_state
{
    FC_BEFORE_START=0,
    FC_BEFORE_START_OVERFLOW1=1,
    FC_BEFORE_START_OVERFLOW2=2,
    FC_BEFORE_START_OVERFLOW3=3,
    FC_BEFORE_START_OVERFLOW4=4,
    FC_FREQ=5,
    FC_FREQ_OVERFLOW1=6,
    FC_FREQ_OVERFLOW2=7,
    FC_FREQ_OVERFLOW3=9,
    FC_FREQ_OVERFLOW4=10,
    FC_ON_CYCLE=11,
    FC_ON_CYCLE_OVERFLOW1=12,
    FC_ON_CYCLE_OVERFLOW2=13,
    FC_ON_CYCLE_OVERFLOW3=14,
    FC_ON_CYCLE_OVERFLOW4=15,
    FC_DONE=16,
    FC_DONE_OVERFLOW1=17,
    FC_DONE_OVERFLOW2=18,
    FC_DONE_OVERFLOW3=19,
    FC_DONE_OVERFLOW4=20,
    FC_DISABLED=21,
};
typedef enum freqcount_state freqcount_state_t;

extern volatile freqcount_state_t freqcount_state;

enum freqcount_average_state
{
    FC_AVERAGE_IN_PROGRESS,
    FC_AVERAGE_DONE
};
typedef enum freqcount_average_state freqcount_average_state_t;

#ifndef FREQCOUNT_NOSLOW_SUPPORT
extern volatile uint8_t timer_overflows;
#endif

#ifdef FREQCOUNT_DUTY_SUPPORT
freqcount_average_state_t freqcount_average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty);
#else
freqcount_average_state_t freqcount_average_results(uint32_t freqcount_ticks);
#endif

#endif /* FREQCOUNT_INTERNAL_H */
