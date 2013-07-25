/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ultrasonic.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef SRF05_SUPPORT
int16_t parse_cmd_srf05_command(char *cmd, char *output, uint16_t len) 
{
  uint16_t distance = srf05_get();
 
  if (distance == 0 ) {
	return ECMD_ERR_PARSE_ERROR;
  }
#if !SRF05_FAKTOR
#error SRF05_FAKTOR has to be non-zero!
#endif
 // only metric values so far
 // metric divisor is 58 for cm
 // imperial divisor is 148 for inches
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%i"), distance / SRF05_FAKTOR ));
}
#endif /* SRF05_SUPPORT */

/*
  -- Ethersex META --
  block([[Ultrasonic]])
  header(hardware/ultrasonic/ultrasonic.h)
  ecmd_ifdef(SRF05_SUPPORT)
    ecmd_feature(srf05_command, "srf05",, Read SRF05 measurement)
  ecmd_endif()
*/
