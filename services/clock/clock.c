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
#include "hardware/i2c/master/i2c_ds13x7.h"
#include "services/ntp/ntp.h"
#include "core/debug.h"
#include "core/periodic.h"
#include "clock.h"
#include "config.h"

static uint32_t timestamp;
static uint8_t ticks;
static uint32_t sync_timestamp;
static uint32_t n_sync_timestamp;
static uint32_t n_sync_tick;
static int16_t delta;
static uint16_t ntp_count;
static uint16_t dcf_count;

#define NTP_RESYNC_PERIOD 1800

#ifdef NTP_SUPPORT
static uint16_t ntp_timer = 1;
#endif

#ifdef WHM_SUPPORT
uint32_t startup_timestamp;
#endif

#if defined(CLOCK_DATETIME_SUPPORT) || defined(DCF77_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)
static uint8_t months[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif

void
clock_init(void)
{
#ifdef CLOCK_CRYSTAL_SUPPORT
	ASSR = _BV(CLOCK_TIMER_AS);
	CLOCK_TIMER_CNT = 0;
	/* 64 prescaler to get every 0.5 second an interrupt */
	CLOCK_TIMER_TCCR = _BV(CLOCK_SELECT_1) | _BV(CLOCK_SELECT_0);

	/* Wait until the bytes are written */
#ifdef CLOCK_TIMER_RBUSY
	while (ASSR & ( _BV(CLOCK_TIMER_NBUSY) | _BV(CLOCK_TIMER_RBUSY))) {}
#else
	while (ASSR & ( _BV(CLOCK_TIMER_NBUSY))) {}
#endif

	/* Clear the interrupt flags */
	CLOCK_TIMER_TIFR &= ~_BV(CLOCK_TIMER_OVERFLOW);

	/* Enable the timer interrupt */
	CLOCK_TIMER_TIMSK |= _BV(CLOCK_TIMER_ENABLE);
#endif

	/* reset dcf_count */
	dcf_count = 0;
}

#if defined(CLOCK_CRYSTAL_SUPPORT) || defined(CLOCK_CPU_SUPPORT)
#ifdef CLOCK_CPU_SUPPORT
SIGNAL(TIMER1_OVF_vect)
#else
SIGNAL(CLOCK_SIG)
#endif
{
#ifdef CLOCK_CPU_SUPPORT
	extern uint8_t milliticks;
	milliticks = 0;

	TCNT1 = 65536-CLOCK_SECONDS;
	OCR1A = 65536-CLOCK_SECONDS+CLOCK_TICKS;
#endif

#if defined(CLOCK_CRYSTAL_SUPPORT)
    /* If we use Crystal Support we have an interrupt every 0.5
       seconds, so we have to drop every second interrupt */
    static uint8_t clock_crystal_interrupt_drop = 0;
    clock_crystal_interrupt_drop ^= 1;
    if (clock_crystal_interrupt_drop)
        return;
#endif

#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
	if (!sync_timestamp || sync_timestamp == timestamp)
#endif
		timestamp++;

	if (sync_timestamp)
		sync_timestamp++;
}
#endif


void
clock_periodic(void)
{
#ifdef NTP_SUPPORT
	if (ntp_timer)
		ntp_timer--;
	else {
		/* Retry in ~10 seconds */
		ntp_timer = 10;
		ntp_send_packet();
	}
#endif
}

void
clock_tick(void)
{
	if (++ticks >= 50) {
		/* Only clock here, when no crystal is connected */
#if !defined(CLOCK_CRYSTAL_SUPPORT) && !defined(CLOCK_CPU_SUPPORT)
		/* Don't wait for a sync, if no sync source is enabled */
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
		if (!sync_timestamp || sync_timestamp == timestamp)
#endif
			timestamp++;

		if (sync_timestamp)
			sync_timestamp++;
#endif /* CLOCK_CRYSTAL_SUPPORT */

		ticks = 0;
	}
}

void
clock_set_time_raw(uint32_t new_sync_timestamp)
{
timestamp=new_sync_timestamp;
}

void
clock_set_time(uint32_t new_sync_timestamp)
{
	/* The clock was synced */
	if (sync_timestamp) {
		delta = new_sync_timestamp - sync_timestamp;
#if defined(CLOCK_NTP_ADJUST_SUPPORT) && !defined(CLOCK_CPU_SUPPORT)
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
#endif  /* CLOCK_NTP_ADJUST_SUPPORT */
	}

	sync_timestamp = new_sync_timestamp;
	n_sync_timestamp = new_sync_timestamp;
	n_sync_tick = TCNT2;

	/* Allow the clock to jump forward, but not to go backward
	 * except the time difference is greater than 5 minutes */
	if (sync_timestamp > timestamp || (timestamp - sync_timestamp) > 300)
		timestamp = sync_timestamp;

#ifdef WHM_SUPPORT
	if (startup_timestamp == 0)
		startup_timestamp = sync_timestamp;
#endif

#ifdef I2C_DS13X7_SUPPORT
        i2c_ds13x7_sync(sync_timestamp);
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

uint8_t
clock_get_ticks(void)
{
	return ticks;
}

uint32_t
clock_last_sync(void)
{
	return n_sync_timestamp;
}

uint32_t
clock_last_s_tick(void)
{
	return n_sync_tick;
}

int16_t
clock_last_delta(void)
{
	return delta;
}

uint16_t
clock_dcf_count(void)
{
	return dcf_count;
}

void
set_dcf_count(uint16_t new_dcf_count)
{
	dcf_count = (new_dcf_count == 0) ? 0 : dcf_count + new_dcf_count;
}

uint16_t
clock_ntp_count(void)
{
	return ntp_count;
}

void
set_ntp_count(uint16_t new_ntp_count)
{
	ntp_count = (new_ntp_count == 0) ? 0 : ntp_count + new_ntp_count;
}

#ifdef NTP_SUPPORT
uint16_t
clock_last_ntp(void)
{
	return ntp_timer;
}
#endif

#ifdef WHM_SUPPORT
uint32_t
clock_get_startup(void)
{
	return startup_timestamp;
}
#endif

#if defined(CLOCK_DATETIME_SUPPORT) || defined(DCF77_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)
uint32_t
clock_utc2timestamp(struct clock_datetime_t *d, uint8_t cest)
{
	uint32_t timestamp;

	/* seconds + minutes */
	timestamp = d->sec + d->min * 60;

	/* hours */
	timestamp += d->hour * 3600UL;

	/* days: don't count last day - it is covered above */
	timestamp += (d->day - 1) * 86400UL;

	/* month: don't count last month - it is covered above */
	for (uint8_t month = d->month; --month >= 1; ) {
		uint8_t monthdays = pgm_read_byte(&months[month-1]);

		/* feb has one day more in a leap year */
		if (month == 2 && is_leap_year(d->year))
			monthdays++;

		timestamp += (monthdays * 86400UL);
	}

	/* year: For every year from EPOCH_YEAR upto now, check for a leap year
	 *
	 * (for details on leap years see http://en.wikipedia.org/wiki/Leap_year )
	 *
	 */
	/* year, check if we have enough days left to fill a year */
	for (uint16_t year = EPOCH_YEAR; year < d->year+1900; year++) {
		timestamp += (is_leap_year(year)) ? 31622400UL : 31536000UL;
	}

	/* convert to UTC depending on CEST */
	timestamp -= (cest == 0) ? 3600UL : 7200UL;

	return timestamp;
}
#endif

#if defined(CLOCK_DATETIME_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)
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

	/* year: For every year from EPOCH_YEAR up to now, check for a leap year
	 *
	 * (for details on leap years see http://en.wikipedia.org/wiki/Leap_year )
	 *
	 */
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
		if (d->month == 2 && is_leap_year(year))
       			monthdays++;

		/* if we have not enough days left to fill this month, we are done */
		if (days < monthdays)
			break;

		days -= monthdays;
		d->month++;
	}
	d->month++;
	d->day = (uint8_t) days+1;
}

#if TIMEZONE == TIMEZONE_CEST
/* This function checks if the last day in month is:
 * -1: in the future
 *  0: today is the last sunday in month
 *  1: in the past
 * This will ONLY work with month.day == 31
 */
int8_t last_sunday_in_month(uint8_t day, uint8_t dow)
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
#endif
	clock_datetime(d, timestamp);
}
#endif
/*
  -- Ethersex META --
  header(services/clock/clock.h)
  init(clock_init)
  timer(1, clock_tick())
  timer(50, clock_periodic())
*/
