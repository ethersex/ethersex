/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Peter Marschall <peter@adpm.de>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "clock.h"
#ifdef DCF77_SUPPORT
#include "hardware/clock/dcf77/dcf77.h"
#endif
#include "clock_ecmd_date.h"

#include "protocols/ecmd/ecmd-base.h"

#if TZ_OFFSET < 0
#define _TZ_FORMAT_STRING " -%d:%02d"
#define _TZ_OFFSET        (-TZ_OFFSET)
#define _DST_OFFSET       (-DST_OFFSET)
#elif TZ_OFFSET > 0
#define _TZ_FORMAT_STRING " +%d:%02d"
#define _TZ_OFFSET        (TZ_OFFSET)
#define _DST_OFFSET       (DST_OFFSET)
#elif DST_OFFSET > 0
#define _TZ_FORMAT_STRING " +%d:%02d"
#define _TZ_OFFSET        (TZ_OFFSET)
#define _DST_OFFSET       (DST_OFFSET)
#else
#define _TZ_FORMAT_STRING " %d:%02d"
#define _TZ_OFFSET        (TZ_OFFSET)
#define _DST_OFFSET       (DST_OFFSET)
#endif

static int16_t
generate_time_string(clock_datetime_t * date, char *output, uint16_t len)
{
  const char *dow = clock_dow_string(date->dow);
  div_t tz = div(_TZ_OFFSET + (date->isdst ? _DST_OFFSET : 0), 60);
  return ECMD_FINAL(snprintf_P(output, len,
                               PSTR("%c%c%c %02d.%02d.%04d %02d:%02d:%02d"
                                    _TZ_FORMAT_STRING
#ifdef CLOCK_DEBUG
                                    " (doy=%d,woy=%d,dst=%d)"
#endif
				    ),
                               pgm_read_byte(dow),
                               pgm_read_byte(dow + 1),
                               pgm_read_byte(dow + 2),
                               date->day,
                               date->month,
                               date->year + 1900,
                               date->hour,
                               date->min,
                               date->sec,
			       tz.quot, tz.rem
#ifdef CLOCK_DEBUG
                               ,date->yday + 1,
                               clock_woy(date->day, date->month, date->year),
                               date->isdst
#endif
			       ));
}

int16_t
parse_cmd_date(char *cmd, char *output, uint16_t len)
{
  clock_datetime_t date;
  clock_current_localtime(&date);
  return generate_time_string(&date, output, len);
}

#ifdef DCF77_SUPPORT
int16_t
parse_cmd_lastdcf(char *cmd, char *output, uint16_t len)
{
  clock_datetime_t date;
  uint32_t last_valid;

  last_valid = dcf77_get_last_valid_timestamp();

  clock_localtime(&date, last_valid);
  return generate_time_string(&date, output, len);
}
#endif

/*
  -- Ethersex META --
  block([[Am_Puls_der_Zeit|Clock]])
  ecmd_feature(date, "date",, Print the current date.)
  ecmd_ifdef(DCF77_SUPPORT)
    ecmd_feature(lastdcf, "lastdcf",, Print when last valid DCF signal was received.)
  ecmd_endif()
*/
