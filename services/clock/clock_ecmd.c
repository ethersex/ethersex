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

#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "clock.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_time(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%lu"), clock_get_time()));
}

int16_t parse_cmd_date(char *cmd, char *output, uint16_t len)
{
  char *weekdays = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
  struct clock_datetime_t date;
  clock_current_localtime(&date);

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%.2d:%.2d:%.2d %.2d.%.2d.%.2d %s"),
                    date.hour, date.min, date.sec, date.day, date.month, date.year,
                    weekdays + date.dow * 4));
}

/*
  -- Ethersex META --
  block(Clock)
  ecmd_feature(time, "time",, Display the current time in seconds since January 1st 1970.)
  ecmd_feature(date, "date",, Display the current date.)
*/
