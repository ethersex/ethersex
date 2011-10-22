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

enum freqcount_state
{
    FC_DISABLED=0,
    FC_BEFORE_START,
    FC_FREQ,
    FC_ON_CYCLE,
    FC_DONE
};
typedef enum freqcount_state freqcount_state_t;

extern volatile uint8_t overflows_since_freq_start;

#ifndef FREQCOUNT_NOSLOW_SUPPORT
extern volatile uint8_t timer_overflows;
#endif

#ifdef FREQCOUNT_DUTY_SUPPORT
void freqcount_average_results(uint32_t freqcount_ticks, uint8_t freqcount_duty);
#else
void freqcount_average_results(uint32_t freqcount_ticks);
#endif

#endif /* FREQCOUNT_INTERNAL_H */
