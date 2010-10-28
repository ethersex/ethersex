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
#ifdef DCF77_SUPPORT
	#include "hardware/clock/dcf77/dcf77.h"
#endif

#include "protocols/ecmd/ecmd-base.h"

static const char const weekdays[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

int16_t parse_cmd_date(char *cmd, char *output, uint16_t len)
{
  struct clock_datetime_t date;
  clock_current_localtime(&date);

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s %02d.%02d.%04d %02d:%02d:%02d %s"),
                               weekdays[date.dow],
                               date.day, date.month, date.year + 1900,
                               date.hour, date.min, date.sec,
#if TIMEZONE == TIMEZONE_CEST
                               datetime_is_CEST(&date) ? "CEST": "CET")
#endif
                               );
}

#ifdef DCF77_SUPPORT
int16_t parse_cmd_lastdcf(char *cmd, char *output, uint16_t len)
{
  struct clock_datetime_t date;
  uint32_t last_valid;

  last_valid = getLastValidDCFTimeStamp();

  clock_localtime(&date, last_valid);


  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s %02d.%02d.%04d %02d:%02d:%02d"),
                               weekdays[date.dow],
                               date.day, date.month, date.year + 1900,
                               date.hour, date.min, date.sec, date.day));
}

#endif

/*
  -- Ethersex META --
  block([[Am_Puls_der_Zeit|Clock]])
  ecmd_feature(date, "date",, Display the current date.)
  ecmd_ifdef(DCF77_SUPPORT)
    ecmd_feature(lastdcf, "lastdcf",, Display when last valid DCF Signal was received.)
  ecmd_endif()
*/
