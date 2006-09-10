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

#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdint.h>
#include <avr/pgmspace.h>

/* structures */
/* {{{ */
struct clock_global_t {
    uint32_t current_time;
};

/* the order for min..dow is the same as in the cron event structure, for
 * easier matching */
struct clock_datetime_t {
    uint8_t sec;
    union {
        uint8_t cron_fields[5];
        struct {
            uint8_t min;
            uint8_t hour;
            uint8_t day;
            uint8_t month;
            uint8_t dow;
        };
    };
    uint8_t year;
};
/* }}} */


/* constants and global variables */
/* {{{ */
extern struct clock_global_t clock_global;
extern uint8_t sntp_monthdays[] PROGMEM;

/* current_time is the amount of seconds since 1.1.1900, 00:00:00 UTC */
#define EPOCH_YEAR 1900
/* 1.1.1900 was a monday */
#define EPOCH_DOW 1

/* }}} */


/* prototypes */

/** do periodic updates, has to be called every second */
void clock_periodic(void);

/** set current time to timestamp */
#define clock_set_time(t) do { \
        clock_global.current_time = t; \
    } while(0);

/** convert time in timestamp to a datetime struct */
void clock_datetime(struct clock_datetime_t *d, uint32_t timestamp);

/** convert current time to a datetime struct */
#define clock_current_datetime(d) clock_datetime(d, clock_global.current_time)

/** test if given year is a leap year */
#define is_leap_year(y)     (((y % 4) == 0) && ( (y % 100 != 0) || (y % 400 == 0) ))

#endif
