/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "core/util/byte2hex.h"

#include "adc.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef ADC_VOLTAGE_SUPPORT
#include <stdio.h>
#include <stdlib.h>
#endif /*ADC_VOLTAGE_SUPPORT */


int16_t
parse_cmd_adc_get(char *cmd, char *output, uint16_t len)
{
  uint16_t adc;
  uint8_t channel;
  uint8_t ret = 0;
  if (cmd[0] && cmd[1])
  {
    channel = cmd[1] - '0';
    if (channel < ADC_CHANNELS)
    {
      adc = adc_get(channel);
      channel = ADC_CHANNELS;
      goto adc_out;
    }
    else
      return ECMD_ERR_PARSE_ERROR;
  }
  for (channel = 0; channel < ADC_CHANNELS; channel++)
  {
    adc = adc_get(channel);
  adc_out:
    output[0] = NIBBLE_TO_HEX(LO4(HI8(adc)));
    output[1] = NIBBLE_TO_HEX(HI4(LO8(adc)));
    output[2] = NIBBLE_TO_HEX(LO4(adc));
    output[3] = ' ';
    output[4] = 0;
    ret += 4;
    output += 4;
  }
  return ECMD_FINAL(ret);
}

#ifdef ADC_VOLTAGE_SUPPORT

int16_t
parse_cmd_adc_vget(char *cmd, char *output, uint16_t len)
{
  uint16_t adc;
  uint8_t channel;
  int16_t plen;
  int16_t ret = 0;
  if (cmd[0] && cmd[1])
  {
    channel = cmd[1] - '0';
    if (channel < ADC_CHANNELS)
    {
      adc = adc_get_voltage(channel);
      channel = ADC_CHANNELS;
      goto adc_out;
    }
    else
      return ECMD_ERR_PARSE_ERROR;
  }
  for (channel = 0; channel < ADC_CHANNELS; channel++)
  {
    adc = adc_get_voltage(channel);
  adc_out:
    plen = snprintf(output, len, "%d ", adc);
    if (plen < 0)
    {
      return ECMD_ERR_PARSE_ERROR;
    }
    len -= plen;
    ret += plen;
    output += plen;
  }
  return ECMD_FINAL(ret);
}

int16_t
parse_cmd_adc_vref(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd == 0)
  {
    return ECMD_FINAL(snprintf(output, len, "%d", adc_get_vref()));
  }

  uint16_t vref = atoi(cmd);
  if (vref < 100 || vref > 6000)
  {
    return ECMD_ERR_PARSE_ERROR;
  }
  adc_set_vref(vref);

  return ECMD_FINAL_OK;
}

#endif /*ADC_VOLTAGE_SUPPORT */

/*
  -- Ethersex META --
  ecmd_feature(adc_get, "adc get", [CHANNEL], Get the ADC value in hex of CHANNEL or if no channel set of all channels.)
  ecmd_ifdef(ADC_VOLTAGE_SUPPORT)
    ecmd_feature(adc_vget, "adc vget", [CHANNEL], Get the ADC value in volt of CHANNEL or if no channel set of all channels.)
    ecmd_feature(adc_vref, "adc vref", [VOLTAGE], Get/Set ADC reference voltage calibration.)
  ecmd_endif()
*/
