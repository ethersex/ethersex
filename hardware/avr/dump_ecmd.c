/*
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "protocols/ecmd/ecmd-base.h"

#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

int16_t
parse_cmd_d(char *cmd, char *output, uint16_t len)
{
  (void) len;

  while (*cmd == ' ')
    cmd++;

  uint16_t temp;
  if (sscanf_P(cmd, PSTR("%x"), &temp) != 1)
    return ECMD_ERR_PARSE_ERROR;

  unsigned char *ptr = (void *) temp;
  for (int i = 0; i < 16; i++)
    sprintf_P(output + (i << 1), PSTR("%02x"), *(ptr++));

  return ECMD_FINAL(32);
}

/*
  -- Ethersex META --
  block(Miscelleanous)
  ecmd_feature(d, "d ", ADDR, Dump the memory at ADDR (16 bytes).)
 */
