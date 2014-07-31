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
parse_cmd_ask_1527_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 10;
  int ret = sscanf_P(cmd, PSTR("%hhu,%hhu,%hhu %hhu %hhu"), &(command[0]),
                     &(command[1]), &(command[2]), &delay, &cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  ask_1527_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[ASK]])
  ecmd_feature(ask_1527_send, "ask 1527", HOUSECODE COMMAND DELAY CNT, )
*/
