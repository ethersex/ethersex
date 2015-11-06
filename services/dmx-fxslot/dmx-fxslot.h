/*
 *
 * Copyright (c) 2011 by Jonas Eickhoff <jonas02401@googlemail.com>
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

#ifndef DMX_FXSLOT_H
#define DMX_FXSLOT_H

#include <stdlib.h>
#include "config.h"

#ifdef DMX_FXSLOT_SUPPORT

#define DMX_FXSLOT_VARIABLE_NUMBER 4
#define DMX_FXSLOT_MAX_DEVICE_CHANNELS 3

#define DMX_FXLIST_RAINBOW 1
#define DMX_FXLIST_RANDOM 2
#define DMX_FXLIST_FIRESIMULATION 3
#define DMX_FXLIST_WATERSIMULATION 4
#define DMX_FXLIST_RGB 5


struct fxslot_struct
{
  uint8_t active;               // fxslot status 0=disabled / 1=enabled
  uint8_t effect;               // which effect is assigned to the fxslot
  uint16_t speed;               // speed of the effect
  uint16_t startchannel;        // startchannel of the fxslot
  uint8_t universe;             // universe of the fxslot
  uint16_t speedcounter;        // variable to control the speed in the 'dmx_fxslot_process'
  uint8_t effect_variable[DMX_FXSLOT_VARIABLE_NUMBER];  // variables you can use in effects
  uint8_t devices;              // amount of devices
  uint8_t margin;               // space between the devices
  uint8_t max_device_channels;  // max amount of channels per device in changed in the effect
  uint8_t device_channel[DMX_FXSLOT_MAX_DEVICE_CHANNELS];       // channels
};

extern struct fxslot_struct fxslot[DMX_FXSLOT_AMOUNT];

//use this struct to save some space in EEPROM
struct fxslot_struct_stripped
{
  uint8_t active;               // fxslot status 0=disabled / 1=enabled
  uint8_t effect;               // which effect is assigned to the fxslot
  uint16_t speed;               // speed of the effect
  uint16_t startchannel;        // startchannel of the fxslot
  uint8_t universe;             // universe of the fxslot
  uint8_t devices;              // amount of devices
  uint8_t margin;               // space between the devices
};


#ifdef DMX_FX_RAINBOW
void dmx_fx_rainbow_colors(uint8_t);
#endif

#ifdef DMX_FX_RANDOM
void dmx_fx_random_colors(uint8_t);
#endif

#ifdef DMX_FX_FIRE
void dmx_fx_firesimulation(uint8_t);
#endif

#ifdef DMX_FX_WATER
void dmx_fx_watersimulation(uint8_t);
#endif

#ifdef DMX_FX_RGB
void dmx_fx_rgb(uint8_t);
#endif

void dmx_fxslot_init(uint8_t);

void dmx_fxslot_setchannels(uint8_t);   // write device_channels to all devices defined by amout of devices and margin
void dmx_fxslot_process(void);

//EEPROM methods
void dmx_fxslot_restore(void);
void dmx_fxslot_save(void);

#endif
#endif // DMX_FXSLOT_H
