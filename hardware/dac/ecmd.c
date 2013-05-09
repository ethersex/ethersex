/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 3 or
 * later) as published by the Free Software Foundation.
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

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

#include "tlc5620.h"


int16_t parse_cmd_tlc5620(char *cmd, char *output, uint16_t len)
{

  uint8_t channel;
  uint8_t value;
  while(*cmd == ' ') cmd++;

  if (sscanf_P(cmd, PSTR("%hhu %hhx"), &channel, &value) != 2)
    return ECMD_ERR_PARSE_ERROR;

  if (channel > 3) channel = 3;

  tlc5620_set_channel(channel, value);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block(Digital/Analog Conversion ([[DAC]]))
  ecmd_feature(tlc5620, "tlc5620 ", `[CHANNEL] [VALUE], Set Output to value (Value: 0-0xff)')
*/
