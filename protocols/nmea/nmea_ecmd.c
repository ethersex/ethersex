/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
#include <string.h>
#include <stdio.h>

#include "protocols/ecmd/ecmd-base.h"
#include "nmea.h"

int16_t
parse_cmd_nmea_get(char *cmd, char *output, uint16_t len) 
{
  if (!nmea_data.valid)
    return snprintf_P (output, len, PSTR("no data available.\n"));

  memmove (output, nmea_data.latitude, 10);
  output[10] = ' ';
  memmove (output + 11, nmea_data.longitude, 11);

  return ECMD_FINAL(22);
}

/*
  -- Ethersex META --
  ecmd_feature(nmea_get, "nmea get")
*/
