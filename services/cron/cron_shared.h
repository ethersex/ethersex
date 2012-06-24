/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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
#include "services/clock/clock.h"

#ifndef _CRON_SHARED_H
#define _CRON_SHARED_H

#define CRON_DAY_SUN _BV(0)
#define CRON_DAY_MON _BV(1)
#define CRON_DAY_TUE _BV(2)
#define CRON_DAY_WED _BV(3)
#define CRON_DAY_THU _BV(4)
#define CRON_DAY_FRI _BV(5)
#define CRON_DAY_SAT _BV(6)

typedef struct
{
  union
  {
    int8_t fields[4];
    /** meaning of the signed values in the following structure:
      *   x in 0..59:    absolute value (minute)
      *   x in 0..23:    absolute value (hour)
      *   x in 0..30:    absolute value (day)
      *   x in 0..12:    absolute value (month)
      *   x is    -1:    wildcard
      *   x in -59..-2:  Example -2 for hour: when hour % 2 == 0 <=> every 2 hours */
    struct
    {
      int8_t minute;
      int8_t hour;
      int8_t day;
      int8_t month;
    };
  };
  /**   x in SUN..SAT: absolute value (dow) // day of the week */
  int8_t daysofweek;
} cron_conditions_t;

extern uint8_t cron_check_event(cron_conditions_t * cond, uint8_t use_utc,
                                clock_datetime_t * d, clock_datetime_t * ld);

#endif /* _CRON_SHARED_H */
