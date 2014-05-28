/*
 *
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

#include <avr/io.h>
#include "config.h"
#include "core/debug.h"
#include "dmx_storage.h"
#ifdef DMX_STORAGE_SUPPORT

static struct dmx_universe dmx_universes[DMX_STORAGE_UNIVERSES];

/* This function searches for a free slot an returns the id */
int8_t
dmx_storage_connect(uint8_t universe)
{
  if (universe < DMX_STORAGE_UNIVERSES)
  {
    for (uint8_t i = 0; i < DMX_STORAGE_SLOTS; i++)
    {
      if (dmx_universes[universe].slots[i].inuse == DMX_SLOT_USED)
        continue;
      else
      {
#ifdef DMX_STORAGE_DEBUG
        debug_printf
          ("DMX STOR: got new connection to universe %d, id is %d\n",
           universe, i);
#endif
        dmx_universes[universe].slots[i].inuse = DMX_SLOT_USED;
        return i;
      }
    }
  }
  return -1;
}

void
dmx_storage_disconnect(uint8_t universe, int8_t slot)
{
  if (universe < DMX_STORAGE_UNIVERSES && slot < DMX_STORAGE_SLOTS &&
      slot >= 0)
    dmx_universes[universe].slots[slot].inuse = DMX_SLOT_FREE;
}

uint8_t
get_dmx_channel(uint8_t universe, uint16_t channel)
{
  if (channel < DMX_STORAGE_CHANNELS
      && universe < DMX_STORAGE_UNIVERSES
      && dmx_universes[universe].universe_state == DMX_LIVE)
    return (dmx_universes[universe].dimmer * dmx_universes[universe].channels[channel]) / 255;
  else
    return 0;
}

uint8_t
get_dmx_channel_raw(uint8_t universe, uint16_t channel)
{
  if (channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
    return dmx_universes[universe].channels[channel];
  else
    return 0;
}

uint8_t
get_dmx_channel_slot(uint8_t universe, uint16_t channel, int8_t slot)
{
  if (slot < DMX_STORAGE_SLOTS && slot >= 0)
    dmx_universes[universe].slots[slot].slot_state = DMX_UNCHANGED;
  return get_dmx_channel(universe, channel);
}

uint8_t
set_dmx_channel(uint8_t universe, uint16_t channel, uint8_t value)
{
#ifdef DMX_STORAGE_DEBUG
  debug_printf
    ("DMX STOR: set dmx_channels: Universe: %d channel: %d value: %d \n",
     universe, channel, value);
#endif
  if (channel < DMX_STORAGE_CHANNELS && universe < DMX_STORAGE_UNIVERSES)
  {
    if (dmx_universes[universe].channels[channel] != value)
    {
      dmx_universes[universe].channels[channel] = value;
      for (uint8_t i = 0; i < DMX_STORAGE_SLOTS; i++)
        dmx_universes[universe].slots[i].slot_state = DMX_NEWVALUES;
    }
    return 0;
  }
  else
  {
    return 1;
  }
}

void
set_dmx_channels(const uint8_t * channel_data, uint8_t universe, uint16_t start_from_channel, uint16_t len)
{
  /* if our input is bigger than our storage */
  if (start_from_channel + len > DMX_STORAGE_CHANNELS)
    len = DMX_STORAGE_CHANNELS - start_from_channel;
#ifdef DMX_STORAGE_DEBUG
  debug_printf("DMX STOR: set dmx_channels: Universe: %d Start %d Length: %d \n",
               universe, start_from_channel, len);
#endif
  if (universe < DMX_STORAGE_UNIVERSES)
  {
    for (uint16_t i = 0; i < len; i++)
    {
      dmx_universes[universe].channels[start_from_channel + i] = channel_data[i];
#ifdef DMX_STORAGE_DEBUG
      debug_printf("DMX STOR: Universe: %d chan: %d value %d \n", universe, i,
                   dmx_universes[universe].channels[i]);
#endif
    }
    for (uint8_t i = 0; i < DMX_STORAGE_SLOTS; i++)
      dmx_universes[universe].slots[i].slot_state = DMX_NEWVALUES;
  }
}

enum dmx_universe_state
get_dmx_universe_state(uint8_t universe)
{
  if (universe < DMX_STORAGE_UNIVERSES)
    return dmx_universes[universe].universe_state;
  else
    return DMX_LIVE;
}

enum dmx_slot_state
get_dmx_slot_state(uint8_t universe, int8_t slot)
{
  if (universe < DMX_STORAGE_UNIVERSES && slot < DMX_STORAGE_SLOTS &&
      slot >= 0)
    return dmx_universes[universe].slots[slot].slot_state;
  else
    return DMX_UNCHANGED;
}

void
dmx_storage_init()
{
  for (uint8_t universe = 0; universe < DMX_STORAGE_UNIVERSES; universe++)
  {
    for (uint8_t slot = 0; slot < DMX_STORAGE_SLOTS; slot++)
    {
      dmx_universes[universe].slots[slot].slot_state = DMX_NEWVALUES;
      dmx_universes[universe].slots[slot].inuse = DMX_SLOT_FREE;
    }
    for (uint16_t channel = 0; channel < DMX_STORAGE_CHANNELS; channel++)
    {
      dmx_universes[universe].channels[channel] = 0;
    }
    dmx_universes[universe].universe_state = DMX_LIVE;
    dmx_universes[universe].dimmer = 255;
  }
}

void
set_dmx_universe_state(uint8_t universe, enum dmx_universe_state state)
{
  if (universe < DMX_STORAGE_UNIVERSES)
  {
    dmx_universes[universe].universe_state = state;
    for (uint8_t i = 0; i < DMX_STORAGE_SLOTS; i++)
      dmx_universes[universe].slots[i].slot_state = DMX_NEWVALUES;
  }
}

uint8_t
get_dmx_universe_dimmer(uint8_t universe)
{
  if (universe < DMX_STORAGE_UNIVERSES)
    return dmx_universes[universe].dimmer;
  else
    return 0;
}

void
set_dmx_universe_dimmer(uint8_t universe, uint8_t value)
{
  if (universe < DMX_STORAGE_UNIVERSES)
  {
    dmx_universes[universe].dimmer = value;
    for (uint8_t i = 0; i < DMX_STORAGE_SLOTS; i++)
      dmx_universes[universe].slots[i].slot_state = DMX_NEWVALUES;
  }
}


#endif
/*
   -- Ethersex META --
   header(services/dmx-storage/dmx_storage.h)
   init(dmx_storage_init)
 */
