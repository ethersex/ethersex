/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../net/ntp_net.h"
#include "../dns/resolv.h"
#include "../ntp/ntp.h"
#include "clock.h"

static uint32_t timestamp = 1;
static uint32_t sync_timestamp = 0;

#ifdef NTP_SUPPORT
static uint16_t ntp_timer = 1;
#endif

static uint8_t months[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void
clock_init(void)
{
#ifdef CLOCK_CRYSTAL_SUPPORT
  ASSR = _BV(AS2);
  TCNT2 = 0;
  /* 120 prescaler to get every second an interrupt */
  TCCR2B = _BV(CS22) | _BV(CS20);
  while(ASSR & ( _BV(TCN2UB) | _BV(TCR2BUB))) {}
  /* Clear the interrupt flags */
  TIFR2 &= ~_BV(TOV2);
  /* Enable the timer interrupt */
  TIMSK2 |= _BV(TOIE2);
#endif
}

#ifdef CLOCK_CRYSTAL_SUPPORT
SIGNAL(SIG_OVERFLOW2)
{
  if(sync_timestamp <= timestamp)
    timestamp ++;
}
#endif


void
clock_tick(void) 
{
#ifdef NTP_SUPPORT
  if(ntp_timer) {
    if((-- ntp_timer) == 0)
      ntp_send_packet();
  }

  if(timestamp <= 50 && (timestamp % 5 == 0))
    ntp_send_packet();
#endif
  /* Only clock here, when no crystal is connected */
#ifndef CLOCK_CRYSTAL_SUPPORT
  if(sync_timestamp <= timestamp)
    timestamp ++;
#endif
}

void
clock_set_time(uint32_t new_sync_timestamp)
{
  /* The clock was synced */
  sync_timestamp = new_sync_timestamp;

  /* Allow the clock to jump forward, but never ever to go backward. */
  if (sync_timestamp > timestamp)
    timestamp = sync_timestamp;

#ifdef NTP_SUPPORT
  ntp_timer = 4096;
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

    d->year = year % 100;
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
