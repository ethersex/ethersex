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

#ifndef FREQCOUNT_NOSLOW_SUPPORT
extern volatile uint8_t timer_overflows;
#endif

#ifdef FREQCOUNT_DUTY_SUPPORT
void freqcount_average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty);
#else
void freqcount_average_results(uint32_t freqcount_ticks);
#endif

#endif /* FREQCOUNT_INTERNAL_H */
