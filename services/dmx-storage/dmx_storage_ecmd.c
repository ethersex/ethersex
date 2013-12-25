/*
 *
 * Copyright (c) 2011-2013 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "dmx_storage.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DMX_STORAGE_SUPPORT
int16_t
parse_cmd_dmx_get_channel(char *cmd, char *output, uint16_t len)
{
  uint16_t ret = 0, channel = 0;
  uint8_t universe = 0;
  if (cmd[0] != 0)
    ret = sscanf_P(cmd, PSTR("%hu %hhu"), &universe, &channel);
  if (ret == 2)
  {
    if (channel >= DMX_STORAGE_CHANNELS)
      return ECMD_ERR_PARSE_ERROR;
    if (universe >= DMX_STORAGE_UNIVERSES)
      return ECMD_ERR_PARSE_ERROR;
    itoa(get_dmx_channel(universe, channel), output, 10);
    return ECMD_FINAL(strlen(output));
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_dmx_set_channels(char *cmd, char *output, uint16_t len)
{
  uint16_t startchannel = 0, channelcounter = 0, blankcounter = 0;
  uint8_t universe = 0, i = 0, value = 0;
  if (cmd[0] != 0)
  {
    while (cmd[i] == ' ')
      i++;
    sscanf_P(cmd, PSTR("%hhu"), &universe);
    i += universe / 10 + 1;
    while (cmd[i] == ' ')
      i++;
    if (strncmp_P(cmd + i, PSTR("off"), 3) == 0)
    {
      set_dmx_universe_state(universe, DMX_BLACKOUT);
    }
    else if (strncmp_P(cmd + i, PSTR("on"), 2) == 0)
    {
      set_dmx_universe_state(universe, DMX_LIVE);
    }
    else if (strncmp_P(cmd + i, PSTR("dimmer"), 6) == 0)
    {
      sscanf_P(cmd+i+6, PSTR("%hhu"), &value);
      set_dmx_universe_dimmer(universe, value);
    }
    else
    {
      i = 0;
      sscanf_P(cmd, PSTR("%hhu %hu"), &universe, &startchannel);
      if (startchannel >= DMX_STORAGE_CHANNELS)
        return ECMD_ERR_PARSE_ERROR;
      if (universe >= DMX_STORAGE_UNIVERSES)
        return ECMD_ERR_PARSE_ERROR;
      while (blankcounter < 3)
      {
        if (cmd[i] == ' ')
          blankcounter++;
        i++;
      }
      while (cmd[i] != '\0')
      {
        /* read and write all values */
        sscanf_P(cmd + i, PSTR(" %hhu"), &value);
        if (set_dmx_channel(universe, startchannel + channelcounter, value))
          return ECMD_ERR_WRITE_ERROR;
        channelcounter++;
        do
        {                       //search for next space
          i++;
          if (cmd[i] == '\0')
            break;
        }
        while (cmd[i] != ' ');
      }
    }
    return ECMD_FINAL_OK;
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_dmx_channels(char *cmd, char *output, uint16_t len)
{
  itoa(DMX_STORAGE_CHANNELS, output, 10);
  return ECMD_FINAL(strlen(output));
}

int16_t
parse_cmd_dmx_universes(char *cmd, char *output, uint16_t len)
{
  itoa(DMX_STORAGE_UNIVERSES, output, 10);
  return ECMD_FINAL(strlen(output));
}

int16_t
parse_cmd_dmx_get_universe(char *cmd, char *output, uint16_t len)
{
  uint16_t ret = 0;
  uint8_t value = 0, universe = 0;
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)       /* indicator flag: real invocation:  0 */
  {
    /* read universe */
    ret = sscanf_P(cmd, PSTR("%hhu"), &universe);
    if (ret != 1 || universe >= DMX_STORAGE_UNIVERSES)
      return ECMD_ERR_PARSE_ERROR;
    cmd[0] = ECMD_STATE_MAGIC;  /* continuing call: 23 */
    cmd[1] = universe;          /* universe */
    cmd[2] = 0xff;              /* reserved for chan */
    cmd[3] = 0xff;              /* reserved for chan */
    if (get_dmx_universe_state(universe) == DMX_LIVE)
      strncpy_P(output, PSTR("LIVE"), 6);
    else
      strncpy_P(output, PSTR("BLACKOUT"), 10);
    return ECMD_AGAIN(strlen(output));
  }
  /* retrieve universe from *cmd */
  universe = cmd[1];
  /* retrieve chan from *cmd. chan is 16 bit. 
   * cmd[1] in 16 bit is cmd[2] and cmd[3] in 8-bit */
  uint16_t chan = *((uint16_t *) (cmd) + 1);
  if (chan == 0xffff)
  {
    /* Output first channel (dimmer) */
    value = get_dmx_universe_dimmer(universe);
    chan = 0;
  }
  else
  {
    /* request value from dmx-storage */
    value = get_dmx_channel_raw(universe, chan);
    chan++;
  }
  /* write the value to *output with leading 0 so that the output 
   * will be like this:
   * 255
   * 044
   * 003
   * 000
   */
  /* ones */
  output[2] = value % 10 + 48;
  value /= 10;
  /* tens */
  output[1] = value % 10 + 48;
  value /= 10;
  /* hundreds */
  output[0] = value % 10 + 48;
  /* Newline to be better parseable with http */
  output[3] = '\n';
  /* terminate string */
  output[4] = '\0';
  ret = 5;
  if (chan < DMX_STORAGE_CHANNELS)
  {
    *((uint16_t *) (cmd) + 1) = chan;
    return ECMD_AGAIN(ret);
  }
  else
  {
    return ECMD_FINAL(ret);
  }
}
#endif
/*
   -- Ethersex META --
   block([[DMX_Storage]] commands)
   ecmd_feature(dmx_get_channel, "dmx get",, Return channel value) 
   ecmd_feature(dmx_set_channels, "dmx set",, Set channel values) 
   ecmd_feature(dmx_channels, "dmx channels",, Get channels per universe) 
   ecmd_feature(dmx_universes, "dmx universes",, Get universes) 
   ecmd_feature(dmx_get_universe, "dmx universe",, Get a whole universe) 
 */
