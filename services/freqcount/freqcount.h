/*
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

#ifndef FREQCOUNT_H
#define FREQCOUNT_H

#ifdef FREQCOUNT_SUPPORT

// the frequency counter runs at full cpu frequency
#define FREQCOUNT_CLOCKFREQ F_CPU

extern volatile uint8_t timer_overflows;

void freqcount_init (void);
void freqcount_init_measure(void);
void freqcount_mainloop(void);

void freqcount_set_state(uint8_t state,uint8_t channel);

uint32_t freqcount_get_freq_ticks(uint8_t channel);
uint32_t freqcount_get_freq_hz(uint8_t channel);

#ifdef FREQCOUNT_DUTY_SUPPORT
uint8_t freqcount_get_duty(uint8_t channel);
#endif

#endif /* FREQCOUNT_SUPPORT */

#endif /* FREQCOUNT_H */
