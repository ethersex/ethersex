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

#ifndef __CLOCK_LIB_H
#define __CLOCK_LIB_H

#include <stdint.h>
#include <avr/pgmspace.h>

/* convert current time to a datetime struct */
#define clock_current_datetime(d)   clock_datetime(d, clock_get_time())
#define clock_current_localtime(d)  clock_localtime(d, clock_get_time())
#define clock_dow_string(dow)       (&clock_weekdays[(dow)*4])
#define clock_month_days(m)         (pgm_read_byte(&clock_monthdays[(m)-1]))

typedef uint32_t timestamp_t;

typedef struct
{
  uint8_t sec;                  /* Sekunden 0-59 */
  union
  {
    uint8_t cron_fields[4];
    struct
    {
      uint8_t min;              /* Minuten 0-59 */
      uint8_t hour;             /* Stunden 0-23 */
      uint8_t day;              /* Tag des Monats 1-31 */
      uint8_t month;            /* Monat 1-12 */
    };
  };
  uint8_t dow;                  /* Tag der Woche 0-6, So-Sa */
  uint8_t year;                 /* Jahr seit 1900 */
  uint16_t yday;                /* Tag des Jahres 0-365 */
  int8_t isdst;                 /* Sommerzeit */
} clock_datetime_t;

/* test if given year is a leap year */
#define IS_LEAP_YEAR(y)  (((y % 4) == 0) && ( (y % 100 != 0) || (y % 400 == 0) ))
/* current_time is the amount of seconds since 1.1.1900, 00:00:00 UTC */
#define EPOCH_YEAR       1970
#define EPOCH_CENTURY    (EPOCH_YEAR-(EPOCH_YEAR%100))
/* 1.1.1970 was a thursday */
#define EPOCH_DOW        4

extern const char clock_weekdays[];
extern const uint8_t clock_monthdays[];
extern const char cmd_date_text[];
extern const char cmd_tz_text[];

void clock_datetime(clock_datetime_t *, timestamp_t);
void clock_localtime(clock_datetime_t *, const timestamp_t);
timestamp_t clock_mktime(clock_datetime_t *, const uint8_t);
uint8_t clock_dow(const uint8_t, const uint8_t, const uint8_t);
uint16_t clock_yday(const uint8_t, const uint8_t, const uint8_t);
uint8_t clock_woy(const uint8_t, const uint8_t, const uint8_t);
void clock_yday2date(const uint16_t, const uint8_t, uint8_t *, uint8_t *);
void clock_reset_dst_change(void);

#endif /* __CLOCK_LIB_H */
