/*
 * Copyright (c) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) Gregor B.
 * Copyright (c) Dirk Pannenbecker
 * Copyright (c) Guido Pannenbecker
 * Copyright (c) Stefan Riepenhausen
 * Copyright (c) 2012-14 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "protocols/radio/ask/ask.h"
#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_ask_intertechno_sl_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint32_t house;
  uint8_t group;
  uint8_t on;
  uint8_t unit;
  uint8_t button;
  int8_t dim = -1;

  int ret = sscanf_P(cmd, PSTR("%lu %hhu %hhu %hhu"), &house,
                     &on, &button, &dim);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  ask_intertechno_sl_send(house, on, button, dim);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[ASK]])
  ecmd_feature(ask_intertechno_sl_send, "ask itsl", HOUSECODE COMMAND BUTTON [DIM] , "Send Command to Intertechno Self learning switches and dimmers. DIM works only with dimmers and values 0-15.")
*/
