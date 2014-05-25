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
parse_cmd_ask_intertechno_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t family;
  uint8_t group;
  uint8_t device;
  uint8_t command;
  int ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"), &family, &group,
                     &device, &command);
  if (ret < 4)
    return ECMD_ERR_PARSE_ERROR;

  ask_intertechno_send(family, group, device, command);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[ASK]])
  ecmd_feature(ask_intertechno_send, "ask intertechno", FAMILY GROUP DEVICE COMMAND, "Send Command to Intertechno switches (with coding wheel). FAMILY: A=1, ...")
*/
