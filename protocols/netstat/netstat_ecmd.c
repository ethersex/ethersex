/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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
#include "netstat.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_netstat(char *cmd, char *output, uint16_t len) 
{
  if (netstat_send()) 
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(cmd, len, PSTR("sending failed")));
}

/*
  -- Ethersex META --
  block([[Netstat]])
  ecmd_feature(netstat, "ns",,update net statistic for public anouncment of currently running ethersex)
*/
