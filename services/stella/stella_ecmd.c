/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "services/cron/cron.h"

#include "protocols/ecmd/ecmd-base.h"
#include "stella.h"

#ifndef TEENSY_SUPPORT
int16_t
parse_cmd_stella_eeprom_store(char *cmd, char *output, uint16_t len)
{
  stella_storeToEEROM();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_stella_eeprom_load(char *cmd, char *output, uint16_t len)
{
  stella_loadFromEEROM();
  return ECMD_FINAL_OK;
}
#endif /* not TEENSY_SUPPORT */

int16_t
parse_cmd_stella_fadestep(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    stella_fade_step = atoi(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(stella_fade_step, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t
parse_cmd_stella_channels(char *cmd, char *output, uint16_t len)
{
  itoa(STELLA_CHANNELS, output, 10);
  return ECMD_FINAL(strlen(output));
}

int16_t
parse_cmd_stella_channel(char *cmd, char *output, uint16_t len)
{
  char f = 0;
  uint8_t ch = 0;
  uint8_t value = 0;

  // following lines same as: sscanf_P(cmd, PSTR("%u %u %c"), &ch, &value, &f);

  while (*cmd && *cmd == ' ')
    cmd++;                      // skip whitespace
  if (!*cmd)
  {
    /* not first argument == return all channels */
    static uint8_t chan = 0;
    uint8_t ret = 0;
    // First return amount of channels with three bytes
    if (chan == 0)
    {
      output[ret++] = ((uint8_t) STELLA_CHANNELS) / 10 + 48;
      output[ret++] = ((uint8_t) STELLA_CHANNELS) % 10 + 48;
      output[ret++] = '\n';
    }
    // return channel values
    value = stella_getValue(chan);
    output[ret + 2] = value % 10 + 48;
    value /= 10;
    output[ret + 1] = value % 10 + 48;
    value /= 10;
    output[ret + 0] = value % 10 + 48;
    ret += 3;
    if (chan < STELLA_CHANNELS - 1)
    {
      chan++;
      return ECMD_AGAIN(ret);
    }
    else
    {
      chan = 0;
      return ECMD_FINAL(ret);
    }
  }
  ch = atoi(cmd);               // save first argument == channel
  while (*cmd && *cmd != ' ')
    cmd++;                      // skip value
  while (*cmd && *cmd == ' ')
    cmd++;                      // skip whitespace
  if (!*cmd)
  {
    /* no second argument -> get value */
    if (ch >= STELLA_CHANNELS)
      return ECMD_ERR_PARSE_ERROR;

    itoa(stella_getValue(ch), output, 10);
    return ECMD_FINAL(strlen(output));
  }

  value = atoi(cmd);

  while (*cmd && *cmd != ' ')
    cmd++;                      // skip value
  while (*cmd && *cmd == ' ')
    cmd++;                      // skip whitespace

  /* third argument == fade step */
  if (*cmd)
  {
    f = *cmd;

    if (f == 's')
      f = STELLA_SET_IMMEDIATELY;
    else if (f == 'f')
      f = STELLA_SET_FADE;
    else if (f == 'y')
      f = STELLA_SET_FLASHY;
  }

  if (ch >= STELLA_CHANNELS)
    return ECMD_ERR_PARSE_ERROR;

  stella_setValue(f, ch, value);

  return ECMD_FINAL_OK;
}


/*
-- Ethersex META --
block([[Stella_Light]] commands)
ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_feature(stella_eeprom_store, "stella store",, Store values in eeprom)
  ecmd_feature(stella_eeprom_load, "stella load",, Load values from eeprom)
ecmd_endif()

ecmd_feature(stella_channels, "channels",, Return stella channel size)
ecmd_feature(stella_channel, "channel", CHANNEL VALUE FUNCTION,Get/Set stella channel to value. Second and third parameters are optional. Function: You may use 's' for instant set, 'f' for fade and 'y' for flashy fade. )
ecmd_feature(stella_fadestep, "fadestep", FADESTEP, Get/Set stella fade step)
*/
