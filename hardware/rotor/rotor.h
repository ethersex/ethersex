/*
*
* Copyright (c) 2009 by Jonny Roeker <dg9oaa@darc.de>
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

#include <stdint.h>
#include "config.h"

#ifdef ROTORHAMLIB_SUPPORT


#define HYSTERESIS 2       /* Hysterese 2 Grad */


void rotor_init(void);
void rotor_periodic(void);

void     rotor_turn(uint8_t rotation, uint8_t isauto);
void     calibrate_azimuth(uint16_t min, uint16_t max);
void     rotor_stop();

void     break_free(uint16_t delay);
void     break_set(uint16_t delay);

uint8_t  get_rotation();
uint16_t get_adc(int channel);

uint8_t  rotation;
int32_t  alpha;
uint16_t preset;
uint16_t speed;
uint8_t  is_auto;

enum {NO, CW, CCW};

#define AUTO   1
#define MANUEL 0

#endif
