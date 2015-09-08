/*
 * Copyright (c) 2015 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#include "protocols/radio/ask/ask.h"
#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_ask_fa20rf_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint32_t device;
  uint8_t repeat = 10;
  int ret = sscanf_P(cmd, PSTR("%lx %hhu"), &device, &repeat);
  if (ret < 1)
    return ECMD_ERR_PARSE_ERROR;

  ask_fa20rf_send(device, repeat);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[ASK]])
  ecmd_feature(ask_fa20rf_send, "ask fa20rf", DEVICE [REPEAT], )
*/
