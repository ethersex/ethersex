/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "clock.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_uptime(char *cmd, char *output, uint16_t len)
{
    uint32_t working_minutes = clock_get_uptime() / 60;
    int d = (int)(working_minutes / 1440);
    int m = (int)(working_minutes % 1440);
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%u days, %02u:%02u"),
    				 d, m/60, m%60));
}

/*
  -- Ethersex META --
  block([[Am_Puls_der_Zeit|Clock]])
  ecmd_feature(uptime, "uptime",, Display ethersex uptime in unix format.)
*/
