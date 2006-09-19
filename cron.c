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

#include "cron.h"

#ifdef DEBUG_CRON
#include "uart.h"
#endif

struct cron_event_t events[] = { { { {-1, -2, -1, -1, -1} } }, /* when hour % 2 == 0 */
                                 { { {51, -1, -1, -1, -1} } }, /* when minute is 51 */
                                 { { {-2, -1, -1, -1, -1} } }, /* when minute % 2 == 0 */
                                 { { {-3, -1, -1, -1, -1} } },
                                 { { {-5, -1, -1, -1, -1} } },
                               };

void cron_periodic(void)
/* {{{ */ {

#   ifdef DEBUG_CRON
    uart_puts_P("cron: checking for events\r\n");
#   endif

    /* convert time to something useful */
    struct clock_datetime_t d;
    clock_current_datetime(&d);

    for (uint8_t i = 0; i < 5; i++) {

        uint8_t r = cron_check_event(&events[i], &d);

        if (r > 0) {
#           ifdef DEBUG_CRON
            uart_puts_P("cron: event ");
            uart_putdecbyte(i);
            uart_puts_P(" matches!\r\n");
#           endif
        }

    }

} /* }}} */

int8_t cron_check_event(struct cron_event_t *event, struct clock_datetime_t *d)
/* {{{ */ {

    for (uint8_t f = 0; f < 5; f++) {

        /* if this field has a wildcard, just go on checking */
        if (event->fields[f] == -1)
            continue;

        /* if this field has an absolute value, check this value, if it does
         * not match, this event does not match */
        if (event->fields[f] >= 0 && event->fields[f] != d->cron_fields[f])
            return 0;

        /* if this field has a step value, extract value and check */
        if (event->fields[f] < 0) {
            uint8_t step = -(event->fields[f]);

            /* if this is not within the steps, this event does not match */
            if ((d->cron_fields[f] % step) != 0)
                return 0;

        }

    }

    /* if all fields match, this event matches */
    return 1;

} /* }}} */

