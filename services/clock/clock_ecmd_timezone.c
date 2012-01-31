/*
 *
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "clock.h"
#include "clock_ecmd_timezone.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_timezone(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len,
                               PSTR("%2hhd:02%hhu "
                                    "%2hhd:02%hhu "
                                    "%hhu.%hhu.%hhu/%hhu "
                                    "%hhu.%hhu.%hhu/%hhu"),
                               clock_tz.utctime/60,
                               clock_tz.utctime%60,
                               clock_tz.dsttime/60,
                               clock_tz.dsttime%60,
                               clock_tz.dstbegin.month,
                               clock_tz.dstbegin.week,
                               clock_tz.dstbegin.dow,
                               clock_tz.dstbegin.hour,
                               clock_tz.dstend.month,
                               clock_tz.dstend.week,
                               clock_tz.dstend.dow,
                               clock_tz.dstend.hour));

}

/*
  -- Ethersex META --
  block([[Am_Puls_der_Zeit|Clock]])
  ecmd_feature(timezone, "timezone", [[-]HH:MM [[-]HH:MM [m.w.d/h [m.w.d/h]]]], Print or set the current timezone.)
*/
