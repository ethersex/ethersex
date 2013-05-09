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

#include "config.h"

#if defined(CLOCK_DATETIME_SUPPORT) || defined(DCF77_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "clock_lib.h"


#define UTCTIME   (uint32_t)pgm_read_dword(&clock_tz.utctime)
#define DSTTIME   (uint32_t)pgm_read_dword(&clock_tz.dsttime)
#define DSTBEGIN  &clock_tz.dstbegin
#define DSTEND    &clock_tz.dstend


const char clock_weekdays[] PROGMEM = {
  'S', 'u', 'n', 0,
  'M', 'o', 'n', 0,
  'T', 'u', 'e', 0,
  'W', 'e', 'd', 0,
  'T', 'h', 'u', 0,
  'F', 'r', 'i', 0,
  'S', 'a', 't', 0
};

const uint8_t clock_monthdays[] PROGMEM =
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


static const uint16_t clock_monthydays[2][13] PROGMEM = {
  /* normal years  */
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  /* leap years  */
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};


typedef struct
{
  uint8_t month;
  uint8_t week;
  uint8_t dow;
  uint8_t hour;
} clock_dst_t;

typedef struct
{
  int32_t utctime;              /* Offset zu UTC in Sekunden */
  int32_t dsttime;              /* Offset zu Normalzeit in Sekunden */
  clock_dst_t dstbegin;
  clock_dst_t dstend;
} clock_timezone_t;

static const clock_timezone_t PROGMEM clock_tz = {
  .utctime = TZ_OFFSET * 60,
  .dsttime = DST_OFFSET * 60,
  .dstbegin = {.month = DST_BEGIN_MONTH,
               .week = DST_BEGIN_WEEK,
               .dow = DST_BEGIN_DOW,
               .hour = DST_BEGIN_HOUR,
               },
  .dstend = {.month = DST_END_MONTH,
             .week = DST_END_WEEK,
             .dow = DST_END_DOW,
             .hour = DST_END_HOUR,
             },
};


typedef struct
{
  timestamp_t when;
  int8_t year;
} dst_change_t;

static dst_change_t dst_change[2] = { {0, -1}, {0, -1} };


void
clock_datetime(clock_datetime_t * d, timestamp_t t)
{
  /* seconds */
  d->sec = t % 60;
  t /= 60;

  /* minutes */
  d->min = t % 60;
  t /= 60;

  /* hours */
  d->hour = t % 24;
  t /= 24;

  /* t/84600 is always <= 51000, so we can crop this to an uint16_t */
  uint16_t days = (uint16_t) t;

  /* day of week */
  d->dow = (days + EPOCH_DOW) % 7;

  /* year: For every year from EPOCH_YEAR upto now, check for a leap year
   * (for details on leap years see http://en.wikipedia.org/wiki/Leap_year) */
  uint16_t year = EPOCH_YEAR;

  /* year, check if we have enough days left to fill a year */
  while (days >= 365)
  {
    if (IS_LEAP_YEAR(year))
    {
      /* special case: leap year is not over after 365 days... */
      if (days == 365)
        break;

      /* default case: leap years have one more day */
      days -= 1;
    }

    /* normal years have 365 days */
    days -= 365;
    year++;
  }

  d->year = year - 1900;

  /* day of year */
  d->yday = days;

  /* month */
  d->month = 0;
  for (;;)
  {
    uint8_t monthdays = clock_month_days(d->month + 1);

    /* feb has one more day in a leap year */
    if (d->month == 1 && IS_LEAP_YEAR(year))
      monthdays++;

    /* if we have not enough days left to fill this month, we are done */
    if (days < monthdays)
      break;

    days -= monthdays;
    d->month++;
  }

  d->month++;
  d->day = (uint8_t) days + 1;

  /* no daylight saving in utc */
  d->isdst = 0;
}


/* http://de.wikipedia.org/wiki/Zellers_Kongruenz
 * http://www.mikrocontroller.net/topic/144905
 */
uint8_t
clock_dow(const uint8_t day, const uint8_t month, const uint8_t year)
{
  int16_t y = year + EPOCH_CENTURY;
  int8_t m = month - 2;
  if (m <= 0)
  {
    m += 12;
    y--;
  }
  return (83 * m / 32 + day + y + y / 4 - y / 100 + y / 400) % 7;
}


uint16_t
clock_yday(const uint8_t day, const uint8_t month, const uint8_t year)
{
  uint16_t yday =
    pgm_read_word(&clock_monthydays[IS_LEAP_YEAR(year)][month - 1]);
  return yday + day - 1;
}


/* Berechnung erfolgt analog DIN 1355, welche besagt:
 * Der erste Donnerstag im neuen Jahr liegt immer in der KW 1.
 * "Woche" ist dabei definiert als [Mo, ..., So].
 *
 * http://www.a-m-i.de/tips/datetime/datetime.php
 */
uint8_t
clock_woy(const uint8_t day, const uint8_t month, const uint8_t year)
{
  int16_t yday = clock_yday(day, month, year);

  /* Berechnen des Wochentags des 1. Januar */
  int16_t yday_1jan = clock_dow(1, 1, year) - 1;

  /* Sonderfälle Freitag und Samstag */
  if (yday_1jan >= 4)
    yday_1jan -= 7;

  /* Sonderfälle "Jahresanfang mit KW - Nummer aus dem Vorjahr" */
  if ((yday + yday_1jan) <= 0)
    return clock_woy(31, 12, year - 1);

  uint8_t week = ((yday + yday_1jan) / 7) + 1;

  /* 53 Kalenderwochen hat grundsätzlich nur ein Jahr, welches mit einem
   * Donnerstag anfängt! In Schaltjahren ist es auch mit einem Mittwoch
   * möglich, z.B. 1992 Andernfalls ist diese KW schon die KW1 des
   * Folgejahres. */
  if (week == 53)
  {
    if ((yday_1jan == 3) || ((yday_1jan == 2) && IS_LEAP_YEAR(year)))
      ;                         /* Das ist korrekt und erlaubt */
    else
      week = 1;                 /* Korrektur des Wertes */
  }

  return week;
}


void
clock_yday2date(const uint16_t yday, const uint8_t year, uint8_t * day,
                uint8_t * month)
{
  const uint16_t *p = clock_monthydays[IS_LEAP_YEAR(year)];
  for (int8_t m = 12; m >= 0; m--)
  {
    uint16_t d = pgm_read_word(p + m);
    if (yday >= d)
    {
      *month = m + 1;
      *day = 1 + yday - d;
      break;
    }
  }
}


static timestamp_t
clock_date_to_timestamp(const uint8_t day, uint8_t month, const uint8_t year)
{
  /* year */
  uint16_t y = year + EPOCH_CENTURY;
  uint16_t days = (y - EPOCH_YEAR) * 365U + ((y - 1) / 4 - EPOCH_YEAR / 4);

  /* day + month */
  days += clock_yday(day, month, year);
  return days * 86400UL;
}


static uint8_t
clock_compute_change(dst_change_t * change, const clock_dst_t * dst_flash,
                     const uint8_t year)
{
  // copy block from flash to stack for faster access
  clock_dst_t dst;
  memcpy_P(&dst, dst_flash, sizeof(clock_dst_t));

  if (change->year == year)
    return 0;

  int8_t day = dst.dow - clock_dow(1, dst.month, year);
  if (day < 0)
    day += 7;

  uint8_t monthdays = clock_month_days(dst.month);
  for (uint8_t i = 1; i < dst.week; i++)
  {
    if (day + 7 >= monthdays)
      break;
    day += 7;
  }
  day++;

  timestamp_t t = clock_date_to_timestamp(day, dst.month, year);
  t -= UTCTIME;
  t += dst.hour * 3600UL;
  change->when = t;             // - UTCTIME + dst->hour * 3600UL;
  change->year = year;
  return 1;
}


static void
clock_tz_compute(const uint8_t year)
{
  clock_compute_change(&dst_change[0], DSTBEGIN, year);
  if (clock_compute_change(&dst_change[1], DSTEND, year))
    dst_change[1].when -= DSTTIME;      /* tz.dstend.hour stores DST time */
}


static uint8_t
clock_is_dst(const timestamp_t t)
{
  if (!DSTTIME)
    return 0;                   /* keine Sommerzeit */

  uint8_t isdst;
  /* We have to distinguish between northern and southern hemisphere. For
   * the latter the daylight saving time ends in the next year. */
  if (dst_change[0].when > dst_change[1].when)
    isdst = (t < dst_change[1].when || t >= dst_change[0].when);
  else
    isdst = (t >= dst_change[0].when && t < dst_change[1].when);

  return isdst;
}


void
clock_reset_dst_change(void)
{
  dst_change[0].year = -1;
  dst_change[1].year = -1;
}


void
clock_localtime(clock_datetime_t * d, const timestamp_t t)
{
  timestamp_t localtime = t + UTCTIME;
  clock_datetime(d, localtime);
  clock_tz_compute(d->year);
  uint8_t isdst = clock_is_dst(t);
  if (isdst)
  {
    localtime += DSTTIME;
    clock_datetime(d, localtime);
    d->isdst = isdst;
  }
}


timestamp_t
clock_mktime(clock_datetime_t * d, const uint8_t islocal)
{
  timestamp_t t = clock_date_to_timestamp(d->day, d->month, d->year);
  t += 3600UL * d->hour;
  t += 60UL * d->min;
  t += d->sec;
  if (islocal)
  {
    t -= UTCTIME;
    if (d->isdst < 0)
    {
      clock_tz_compute(d->year);
      d->isdst = clock_is_dst(t);
    }
    if (d->isdst > 0)
      t -= DSTTIME;
  }
  d->dow = clock_dow(d->day, d->month, d->year);

  return t;                     /* UTC */
}

#endif // CLOCK_DATETIME_SUPPORT || DCF77_SUPPORT || CLOCK_DATE_SUPPORT || CLOCK_TIME_SUPPORT

