/*
 *
 * Copyright (c) 2011 by Jonas Eickhoff <jonas02401@googlemail.com>
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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

/* Module description: the dmx fxslot module can alter dmx information to create animations and effects for
   headless setups or currently not animated devices */
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "core/eeprom.h"
#include "core/debug.h"
#include "dmx-fxslot.h"
#include "services/dmx-storage/dmx_storage.h"

#ifdef DMX_FXSLOT_SUPPORT
struct fxslot_struct fxslot[DMX_FXSLOT_AMOUNT];
/*
   add new effect function here (don't forget to add in in dmx_fxslot_process as new case)
   you can use variable for effects in fxslot[number].variable[]
   max variables are defiened in dmx-fxslot.h 'DMX_FXSLOT_VARIABLE_NUMBER'
 */

#ifdef DMX_FX_RAINBOW
void
dmx_fx_rainbow_colors(uint8_t fxslot_number)
{
  switch (fxslot[fxslot_number].effect_variable[3])
  {
    case 0:
      if (fxslot[fxslot_number].device_channel[0] > 1)
      {
        fxslot[fxslot_number].device_channel[0]--;
        fxslot[fxslot_number].device_channel[1]++;
      }
      else
      {
        fxslot[fxslot_number].effect_variable[3]++;
      }
      break;
    case 1:
      if (fxslot[fxslot_number].device_channel[1] > 1)
      {
        fxslot[fxslot_number].device_channel[1]--;
        fxslot[fxslot_number].device_channel[2]++;
      }
      else
      {
        fxslot[fxslot_number].effect_variable[3]++;
      }
      break;
    case 2:
      if (fxslot[fxslot_number].device_channel[2] > 1)
      {
        fxslot[fxslot_number].device_channel[2]--;
        fxslot[fxslot_number].device_channel[0]++;
      }
      else
      {
        fxslot[fxslot_number].effect_variable[3] = 0;
      }
      break;
  }
  dmx_fxslot_setchannels(fxslot_number);
}
#endif /*Rainbow end */

#ifdef DMX_FX_RANDOM
void
dmx_fx_random_colors(uint8_t fxslot_number)
{
  fxslot[fxslot_number].device_channel[0] = rand() % 256;
  fxslot[fxslot_number].device_channel[1] = rand() % 256;
  fxslot[fxslot_number].device_channel[2] = rand() % 256;

  dmx_fxslot_setchannels(fxslot_number);
}
#endif /*Random end */

#ifdef DMX_FX_FIRE
void
dmx_fx_firesimulation(uint8_t fxslot_number)
{
  fxslot[fxslot_number].device_channel[0] = 150 + rand() % 64;
  fxslot[fxslot_number].device_channel[1] = 69 + rand() % 32;
  fxslot[fxslot_number].device_channel[2] = 0;

  dmx_fxslot_setchannels(fxslot_number);
}
#endif /*Fire end */

#ifdef DMX_FX_WATER
void
dmx_fx_watersimulation(uint8_t fxslot_number)
{
  /* Blue */
  if (fxslot[fxslot_number].effect_variable[0] == 0 &&
      fxslot[fxslot_number].effect_variable[1] == 0)
  {
    if (rand() % 2)
      fxslot[fxslot_number].effect_variable[3] = 1;     /* We will add some red */
    else
      fxslot[fxslot_number].effect_variable[3] = 2;     /* We will add some green */
  }
  /* As red as it will get.. */
  if (fxslot[fxslot_number].effect_variable[0] == 21)
    fxslot[fxslot_number].effect_variable[3] = 3;       /* Decrease red, return to blue */
  /* As green as it will get.. */
  if (fxslot[fxslot_number].effect_variable[1] == 170)
    fxslot[fxslot_number].effect_variable[3] = 4;       /* Decrease green, return to blue */

  if (fxslot[fxslot_number].effect_variable[3] == 1)
    fxslot[fxslot_number].effect_variable[0]++;
  if (fxslot[fxslot_number].effect_variable[3] == 2)
    fxslot[fxslot_number].effect_variable[1]++;
  if (fxslot[fxslot_number].effect_variable[3] == 3)
    fxslot[fxslot_number].effect_variable[0]--;
  if (fxslot[fxslot_number].effect_variable[3] == 4)
    fxslot[fxslot_number].effect_variable[1]--;

  uint8_t brightness = rand() % 5;

  if (fxslot[fxslot_number].effect_variable[0] - brightness >= 0)
    fxslot[fxslot_number].device_channel[0] =
      fxslot[fxslot_number].effect_variable[0] - brightness;
  else
    fxslot[fxslot_number].device_channel[0] = 0;
  if (fxslot[fxslot_number].effect_variable[1] - brightness >= 0)
    fxslot[fxslot_number].device_channel[1] =
      fxslot[fxslot_number].effect_variable[1] - brightness;
  else
    fxslot[fxslot_number].device_channel[1] = 0;
  /* Blue is (for now) always 100% ; 255 */
  fxslot[fxslot_number].device_channel[2] =
    fxslot[fxslot_number].effect_variable[2] - brightness;

  dmx_fxslot_setchannels(fxslot_number);
}
#endif /*Water end */

#ifdef DMX_FX_RGB
void
dmx_fx_rgb(uint8_t fxslot_number)
{
  switch (fxslot[fxslot_number].effect_variable[0])
  {
    case(1):
      fxslot[fxslot_number].device_channel[0] = 0;
      fxslot[fxslot_number].device_channel[1] = 255;
      fxslot[fxslot_number].device_channel[2] = 0;
      fxslot[fxslot_number].effect_variable[0] = 2;
      break;
    case(2):
      fxslot[fxslot_number].device_channel[0] = 0;
      fxslot[fxslot_number].device_channel[1] = 0;
      fxslot[fxslot_number].device_channel[2] = 255;
      fxslot[fxslot_number].effect_variable[0] = 0;
      break;
    case(0):
    default:
      fxslot[fxslot_number].device_channel[0] = 255;
      fxslot[fxslot_number].device_channel[1] = 0;
      fxslot[fxslot_number].device_channel[2] = 0;
      fxslot[fxslot_number].effect_variable[0] = 1;
      break;
   }
   dmx_fxslot_setchannels(fxslot_number);
}
#endif

void
dmx_fxslot_init(uint8_t fxslot_number)
{
  switch (fxslot[fxslot_number].effect)
  {
      /*
       * add new effect inits as new case:
       *
       * you have to to define the channels per device changed through the effect by:
       * fxslot[fxslot_number].max_device_channels='number of channels'
       */

#ifdef DMX_FX_RAINBOW           //Rainbow init
    case DMX_FXLIST_RAINBOW:
      fxslot[fxslot_number].max_device_channels = 3;
      fxslot[fxslot_number].device_channel[0] = 255;
      fxslot[fxslot_number].device_channel[1] = 0;
      fxslot[fxslot_number].device_channel[2] = 0;
      fxslot[fxslot_number].effect_variable[3] = 0;
      break;
#endif

#ifdef DMX_FX_RANDOM
    case DMX_FXLIST_RANDOM:    //Random init
      fxslot[fxslot_number].max_device_channels = 3;
      break;
#endif

#ifdef DMX_FX_FIRE
    case DMX_FXLIST_FIRESIMULATION:    //Firesimulation init
      fxslot[fxslot_number].max_device_channels = 3;
      break;
#endif

#ifdef DMX_FX_WATER
    case DMX_FXLIST_WATERSIMULATION:   //Watersimulation init
      fxslot[fxslot_number].max_device_channels = 3;
      fxslot[fxslot_number].effect_variable[0] = 0;
      fxslot[fxslot_number].effect_variable[1] = 0;
      fxslot[fxslot_number].effect_variable[2] = 255;
      fxslot[fxslot_number].effect_variable[3] = 0;
      break;
#endif
#ifdef DMX_FX_RGB
    case DMX_FXLIST_RGB:
      fxslot[fxslot_number].max_device_channels = 3;
      fxslot[fxslot_number].effect_variable[0] = 0;
      break;
#endif
  }
  fxslot[fxslot_number].speedcounter = fxslot[fxslot_number].speed;
}

void
dmx_fxslot_process()
{
  for (uint8_t i = 0; i < DMX_FXSLOT_AMOUNT; i++)
  {
    if (fxslot[i].active)
    {
      if (fxslot[i].speedcounter++ >= fxslot[i].speed)
      {
        switch (fxslot[i].effect)
        {

            /*
             * add new effect functions as new case: 
             */

#ifdef DMX_FX_RAINBOW
          case DMX_FXLIST_RAINBOW:     //Rainbow
            dmx_fx_rainbow_colors(i);
            break;
#endif

#ifdef DMX_FX_RANDOM
          case DMX_FXLIST_RANDOM:      //Random
            dmx_fx_random_colors(i);
            break;
#endif

#ifdef DMX_FX_FIRE
          case DMX_FXLIST_FIRESIMULATION:      //Firesimulation
            dmx_fx_firesimulation(i);
            break;
#endif

#ifdef DMX_FX_WATER
          case DMX_FXLIST_WATERSIMULATION:     //Watersimulation
            dmx_fx_watersimulation(i);
            break;
#endif
#ifdef DMX_FX_RGB
          case DMX_FXLIST_RGB:
            dmx_fx_rgb(i);
            break;
#endif
        }
        fxslot[i].speedcounter = 0;
      }
    }

  }
}

void
dmx_fxslot_setchannels(uint8_t fxslot_number)
{
  uint16_t act_channel = fxslot[fxslot_number].startchannel;
  uint8_t universe = fxslot[fxslot_number].universe;
  for (uint8_t i = 0; i < fxslot[fxslot_number].devices; i++)
  {
    for (uint8_t j = 0; j < fxslot[fxslot_number].max_device_channels; j++)
    {
      set_dmx_channel(universe, act_channel,
                      fxslot[fxslot_number].device_channel[j]);
      act_channel++;
    }
    act_channel = act_channel + fxslot[fxslot_number].margin;
  }
}

void
dmx_fxslot_restore()
{
  struct fxslot_struct_stripped fxslots_temp[DMX_FXSLOT_AMOUNT] =
    { {0, 0, 0, 0, 0, 0, 0} };
  eeprom_restore(dmx_fxslots, fxslots_temp,
                 DMX_FXSLOT_AMOUNT * sizeof(struct fxslot_struct_stripped));
  //Stripped structs have been restored, now copy the values
  for (uint8_t i = 0; i < DMX_FXSLOT_AMOUNT; i++)
  {
    fxslot[i].active = fxslots_temp[i].active;
    fxslot[i].effect = fxslots_temp[i].effect;
    fxslot[i].speed = fxslots_temp[i].speed;
    fxslot[i].startchannel = fxslots_temp[i].startchannel;
    fxslot[i].universe = fxslots_temp[i].universe;
    fxslot[i].devices = fxslots_temp[i].devices;
    fxslot[i].margin = fxslots_temp[i].margin;
    dmx_fxslot_init(i);
  }

}

void
dmx_fxslot_save()
{
  //Strip down channels to save space
  struct fxslot_struct_stripped fxslots_temp[DMX_FXSLOT_AMOUNT] =
    { {0, 0, 0, 0, 0, 0, 0} };
  for (uint8_t i = 0; i < DMX_FXSLOT_AMOUNT; i++)
  {
    fxslots_temp[i].active = fxslot[i].active;
    fxslots_temp[i].effect = fxslot[i].effect;
    fxslots_temp[i].speed = fxslot[i].speed;
    fxslots_temp[i].startchannel = fxslot[i].startchannel;
    fxslots_temp[i].universe = fxslot[i].universe;
    fxslots_temp[i].devices = fxslot[i].devices;
    fxslots_temp[i].margin = fxslot[i].margin;
  }
  //fxslots have been stripped, now save them
  eeprom_save(dmx_fxslots, fxslots_temp,
              DMX_FXSLOT_AMOUNT * sizeof(struct fxslot_struct_stripped));
  eeprom_update_chksum();
}

#endif
/*
   -- Ethersex META --
   header(services/dmx-fxslot/dmx-fxslot.h)
   timer(2,dmx_fxslot_process())
   init(dmx_fxslot_restore)
   ifdef(`conf_DMX_FXSLOT_AUTORESTORE',`init(dmx_fxslot_restore)')
 */
