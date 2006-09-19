/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include "clock.h"
#include "cron.h"

#ifdef DEBUG_CLOCK
#include "uart.h"
#endif

struct clock_global_t clock_global;
uint8_t months[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

void clock_periodic(void)
/* {{{ */ {

    /* pass time */
    clock_global.current_time++;

#   ifdef DEBUG_CLOCK
    uart_puts_P("clock: timestamp: 0x");

    uint8_t *p = ((uint8_t *)&clock_global.current_time) + 4;
    for (uint8_t i = 0; i < 4; i++)
        uart_puthexbyte(*(--p));

    uart_eol();

#   endif

    if (clock_global.current_time % 60 == 0) {

#       ifdef DEBUG_CLOCK
        uart_puts_P("clock: one minute passed, time is ");

        struct clock_datetime_t d;
        clock_current_datetime(&d);
        uart_putdecbyte(d.hour);
        uart_putc(':');
        uart_putdecbyte(d.min);
        uart_putc(':');
        uart_putdecbyte(d.sec);
        uart_putc(' ');
        uart_putdecbyte(d.day);
        uart_putc('.');
        uart_putdecbyte(d.month);
        uart_putc('.');
        uart_putdecbyte(d.year);
        uart_putc(' ');
        uart_putdecbyte(d.dow);

        uart_eol();
#       endif

        cron_periodic();

    }

} /* }}} */

void clock_datetime(struct clock_datetime_t *d, uint32_t timestamp)
/* {{{ */ {

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

} /* }}} */
