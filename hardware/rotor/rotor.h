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
#include <stdbool.h>

#ifdef ROTORHAMLIB_SUPPORT


void     rotor_init(void);
void     rotor_periodic(void);

void     rotor_turn(uint8_t rotation, uint8_t isauto);
void     calibrate_azimuth(uint16_t min, uint16_t max);
void     rotor_stop(uint8_t);
void     rotor_park();

void     break_free(uint16_t delay);
void     break_set();

uint8_t  get_az_movement();
uint16_t get_adc(int channel);

struct rot_t {
   /* azimuth values */
   uint8_t az_movement;  // direction of movement
   int16_t az_preset;    // desired target
   int16_t azimuth;      // current angle of rotation
   int16_t az_value;     // current value of rotation
   uint16_t az_min_store, az_max_store;

   /* elevation values */
   uint8_t el_movement;  // direction of movement
   int16_t el_preset;    // desired target
   int16_t elevation;    // current angle of elevation
   int16_t el_value;     // current value of rotation
   uint16_t el_min_store, el_max_store;
   
   uint8_t  is_auto;
   uint16_t speed;
};

struct job_t {
   bool az_haswork;
   bool az_finish;
   bool az_stop_immediately;
   bool az_stop;
   bool az_turn;
};

extern volatile struct rot_t rot;
extern volatile struct job_t job;


#define AUTO   1
#define MANUEL 0

#define NOTDEV 0
#define NO     1
#define CW     2
#define CCW    3


#endif
