/*
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "services/ntp/ntp.h"
#include "core/debug.h"
#include "clock.h"
#include "config.h"

static uint32_t timestamp;
static uint8_t ticks;
static uint32_t sync_timestamp;

#define NTP_RESYNC_PERIOD 1800

#ifdef NTP_SUPPORT
static uint16_t ntp_timer = 1;
#endif

#ifdef WHM_SUPPORT
uint32_t startup_timestamp;
#endif

#ifdef CLOCK_DATETIME_SUPPORT
static uint8_t months[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif

void
clock_init(void)
{
#ifdef CLOCK_CRYSTAL_SUPPORT
  ASSR = _BV(AS2);
  TCNT2 = 0;
  /* 120 prescaler to get every second an interrupt */
  _TCCR2_PRESCALE = _BV(CS22) | _BV(CS20);
#ifdef TCR2BUB
  while(ASSR & ( _BV(TCN2UB) | _BV(TCR2BUB))) {}
#else
  while(ASSR & ( _BV(TCN2UB))) {}
#endif
  /* Clear the interrupt flags */
  _TIFR_TIMER2 &= ~_BV(TOV2);
  /* Enable the timer interrupt */
  _TIMSK_TIMER2 |= _BV(TOIE2);
#endif
}

#ifdef CLOCK_CRYSTAL_SUPPORT
SIGNAL(SIG_OVERFLOW2)
{
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
  if (!sync_timestamp || sync_timestamp == timestamp)
#endif
    timestamp ++;

  if (sync_timestamp)
    sync_timestamp ++;
}
#endif


void
clock_periodic(void)
{
	#ifdef NTP_SUPPORT
	if(ntp_timer)
		ntp_timer--;
	else
	{
	  /* Retry in ~10 seconds */
	  ntp_timer = 10;
	  ntp_send_packet();
	}
	#endif
}

void
clock_tick(void)
{
  if(++ticks > 50){
  /* Only clock here, when no crystal is connected */
#ifndef CLOCK_CRYSTAL_SUPPORT
# /* Don't wait for a sync, if no sync source is enabled */
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
    if(!sync_timestamp || sync_timestamp == timestamp)
#endif
      timestamp ++;

    if (sync_timestamp)
      sync_timestamp ++;

#endif /* CLOCK_CRYSTAL_SUPPORT */

    ticks = 0;
  }
}

void
clock_set_time(uint32_t new_sync_timestamp)
{
#ifdef CLOCK_NTP_ADJUST_SUPPORT
	/* The clock was synced */
	if (sync_timestamp) {
		int16_t delta = new_sync_timestamp - sync_timestamp;
		NTPADJDEBUG ("sync timestamp delta is %d\n", delta);
		if (delta < -300 || delta > 300)
			NTPADJDEBUG ("eeek, delta too large. "
				     "not adjusting.\n");

		else if (sync_timestamp != timestamp)
			NTPADJDEBUG ("our clock is not up with ntp clock.\n");

		else if (NTP_RESYNC_PERIOD == -delta)
			NTPADJDEBUG ("-delta equals resync period, eeek!? "
				     "clock isn't running at all.\n");
		else {
			uint32_t new_value = OCR1A;
			new_value *= NTP_RESYNC_PERIOD;
			new_value /= NTP_RESYNC_PERIOD + delta;

			NTPADJDEBUG ("new OCR1A value %d\n", new_value);
			OCR1A = new_value;
		}
	}
#endif  /* CLOCK_NTP_ADJUST_SUPPORT */

	sync_timestamp = new_sync_timestamp;

	/* Allow the clock to jump forward, but not to go backward
	 * except the time difference is greater than 5 minutes */
	if (sync_timestamp > timestamp || (timestamp - sync_timestamp) > 300)
		timestamp = sync_timestamp;

	#ifdef WHM_SUPPORT
	if (startup_timestamp == 0)
		startup_timestamp = sync_timestamp;
	#endif

	#ifdef NTP_SUPPORT
	ntp_timer = NTP_RESYNC_PERIOD;
	#endif
}

uint32_t
clock_get_time(void)
{
  return timestamp;
}

uint32_t
clock_last_sync(void)
{
  return sync_timestamp;
}

#ifdef WHM_SUPPORT
uint32_t
clock_get_startup(void)
{
  return startup_timestamp;
}
#endif

#ifdef CLOCK_DATETIME_SUPPORT
void
clock_datetime(struct clock_datetime_t *d, uint32_t timestamp)
{

    /* seconds */
    d->sec = timestamp % 60;
    timestamp /= 60;

    /* minutes */
    d->min = timestamp % 60;
    timestamp /= 60;

    /* hours */
    d->hour = timestamp % 24;
    timestamp /= 24;

    /* timestamp/84600 is always <= 51000, so we can crop this to an uint16_t */
    uint16_t days = (uint16_t)timestamp;

    /* day of week */
    d->dow = (days + EPOCH_DOW) % 7;

    /* year: For every year from EPOCH_YEAR upto now, check for a leap year
     *
     * (for details on leap years see http://en.wikipedia.org/wiki/Leap_year )
     *
     * */
    uint16_t year = EPOCH_YEAR;

    /* year, check if we have enough days left to fill a year */
    while (days >= 365) {

        if (is_leap_year(year)) {

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
    d->month = 0;

    /* month */
    while (1) {

        uint8_t monthdays = pgm_read_byte(&months[d->month]);

        /* feb has one more day in a leap year */
        if ( d->month == 2 && is_leap_year(year))
            monthdays++;

        /* if we have not enough days left to fill this month, we are done */
        if (days < monthdays)
            break;

        days -= monthdays;
        d->month++;

    }

    d->month++;
    d->day = (uint8_t)days+1;

}

uint32_t
clock_utc2timestamp(struct clock_datetime_t *d, uint8_t cest)
{
  uint32_t timestamp;
  /* seconds */
  timestamp = d->sec ;

  /* minutes */
  timestamp += d->min * 60;

  /* hours */
  timestamp += d->hour * 3600ULL;

  /* days */
  timestamp += d->day * 86400ULL - 86400ULL;

  /* month */
  while (1) {

    d->month--;

    if ( d->month < 1 )
      break;

    uint8_t monthdays = pgm_read_byte(&months[d->month-1]);

    /* feb has one more day in a leap year */
    if ( d->month == 2 && is_leap_year(d->year))
      monthdays++;

    timestamp = timestamp + (monthdays * 86400ULL);

  }

  /* year: For every year from EPOCH_YEAR upto now, check for a leap year
  *
  * (for details on leap years see http://en.wikipedia.org/wiki/Leap_year )
  *
  * */
  uint16_t year = EPOCH_YEAR;

  /* year, check if we have enough days left to fill a year */
  while (year < d->year+2000) {
    if (is_leap_year(year)) {
      timestamp += 31622400ULL;
    } else {
      timestamp += 31536000ULL;
    }
    year++;
  }
  if (cest == 0)
    timestamp -= 3600ULL;
  else
    timestamp -= 7200ULL;

  return timestamp;
}

#if TIMEZONE == TIMEZONE_CEST
/* This function checks if the last day in month is:
 * -1: in the future
 *  0: today is the last sunday in month
 *  1: in the past
 * This will ONLY work with month.day == 31
 */
static int8_t last_sunday_in_month(uint8_t day, uint8_t dow)
{
  uint8_t last_sunday = day;
  if (last_sunday > dow)
    last_sunday -= dow + 1;

  if ((31 - last_sunday) <= 7) {
    if ((dow % 7) == 0) return 0;
    return 1;
  }
  return -1;
}
#endif

void
clock_localtime(struct clock_datetime_t *d, uint32_t timestamp)
{
#if TIMEZONE == TIMEZONE_CEST
  clock_datetime(d, timestamp);
  /* We must determine, if we have CET or CEST */
  int8_t last_sunday = last_sunday_in_month(d->day, d->dow);
  /* march until october can be summer time */
  if (d->month < 3 || d->month > 10) {
    timestamp += 3600;
  } else if (d->month == 3 && (last_sunday == -1 || (last_sunday == 0 && d->hour < 1))) {
    timestamp += 3600;
  } else if (d->month == 10 && (last_sunday == 1 || (last_sunday == 0 && d->hour > 1))) {
    timestamp += 3600;
  } else {
    timestamp += 7200;
  }
  clock_datetime(d, timestamp);
#else
  clock_datetime(d, timestamp);
#endif
}
#endif
/*
  -- Ethersex META --
  header(services/clock/clock.h)
  init(clock_init)
  timer(1, clock_tick())
  timer(50, clock_periodic())
*/
