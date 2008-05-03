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

#ifndef _CRON_H
#define _CRON_H

#include <stdint.h>
#include "../clock/clock.h"

/* structures */
/* {{{ */


/* meaning of the signed values in cron_event_t (eg for minute):
 *   x in 0..59:    absolute value (minute)
 *   x is -1:       * (wildcard)
 *   x in -59..-2:  * /(-x) (step value (-x), 1 < -x < 60)
 */

struct cron_event_t {
    union{
        int8_t fields[5];
        struct {
            int8_t minute;
            int8_t hour;
            int8_t day;
            int8_t month;
            int8_t dow;
        };
    };
    void (*handler)(void);
};

/* }}} */


/* constants and global variables */
/* {{{ */

/* }}} */


/* prototypes */

/** periodically check, if an event matches the current time. must be called
  * once per minute */
void cron_periodic(void);

/** check if this event matches the current time */
uint8_t cron_check_event(struct cron_event_t *event, struct clock_datetime_t *d);

#endif
