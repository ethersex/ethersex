/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 Mirko Taschenberger <mirkiway at gmx.de>
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
#include "hardware/adc/ads7822/ads7822.h"
#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_ads_get(char *cmd, char *output, uint16_t len)
{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%04x"),get_ads()));
}

int16_t parse_cmd_ads_mean(char *cmd, char *output, uint16_t len)
{
  uint8_t count;
  uint32_t sum=0;

  sscanf_P(cmd, PSTR("%hhu"), &count);
  if (count < 1 || count > 15) return ECMD_ERR_PARSE_ERROR;

  for ( uint16_t i=0; i < (1<<count); i++) {
	sum += get_ads();
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%04x"), (sum>>count)&0xffff));
}

/*
  -- Ethersex META --
  block([[ADS]])
  ecmd_feature(ads_get, "ads get",, Get the ADC value in hex.)
  ecmd_feature(ads_mean, "ads mean", [COUNT], Get the mean of power of 2 COUNT ADC values in hex.)
*/
