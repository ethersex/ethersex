/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Peter Marschall <peter@adpm.de>
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
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "clock.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_time(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  if (*cmd == '\0')
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%lu"), clock_get_time()));
  else {
    uint32_t newtime = strtoul(cmd, NULL, 10);

    if (!newtime)
      return ECMD_ERR_PARSE_ERROR;
    clock_set_time(newtime);
    return ECMD_FINAL_OK;
  }
}

/*
  -- Ethersex META --
  block([[Am_Puls_der_Zeit|Clock]])
  ecmd_feature(time, "time",[UNIXTIME], Display/Set the current time in seconds since January 1st 1970.)
*/
