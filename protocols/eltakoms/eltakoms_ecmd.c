/*
 * Copyright (c) 2021 by Frank Sautter <ethersix@sautter.com>
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

#include "config.h"

#ifdef ELTAKOMS_SUPPORT

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocols/ecmd/ecmd-base.h"
#include "eltakoms.h"


int16_t
parse_cmd_weather(char *cmd, char *output, uint16_t len)
{
  if(eltakoms_data.valid)
    return ECMD_FINAL(snprintf_P(output, len,
      PSTR("t%+3.3d s%2.2d w%2.2d e%2.2d %c d%3.3d v%3.3d %c"),
      eltakoms_data.temperature, eltakoms_data.suns, eltakoms_data.sunw,
      eltakoms_data.sune, (eltakoms_data.obscure ? 'O' : 'o'),
      eltakoms_data.dawn, eltakoms_data.wind,
      (eltakoms_data.rain ? 'R' : 'r')));
  else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("eltakoms data invalid")));
}
#endif /* ELTAKOMS_H */

/*
  -- Ethersex META --
  block([[Eltako]])
  ecmd_feature(weather, "weather",,Get eltako weather data)
*/
