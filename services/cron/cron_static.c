/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include "cron_static.h"
#include "config.h"
#include "mcuf/mcuf.h"
#include "services/clock/clock.h"
#include "protocols/netstat/netstat.h"

uint32_t last_check;

#ifdef  MCUF_CLOCK_SUPPORT
void
mcuf_clock(void)
{
  mcuf_show_clock(1);
}
#endif /* MCUF_CLOCK_SUPPORT */

#ifdef MCUF_MODUL_CRON_SUPPORT
#include "mcuf/mcuf_modul.h"
void
mcuf_modul(void)
{
#ifdef MCUF_MODUL_DISPLAY_MODE_CRON_RANDOM
  mcuf_play_modul(MCUF_MODUL_PLAY_MODE_RANDOM, 0);
#endif
#ifdef MCUF_MODUL_DISPLAY_MODE_CRON_SEQUENCE
  mcuf_play_modul(MCUF_MODUL_PLAY_MODE_SEQUENCE, 0);
#endif
}
#endif // MCUF_MODUL_CRON_SUPPORT

#ifdef TANKLEVEL_CRON_SUPPORT
#include "services/tanklevel/tanklevel.h"
#endif // TANKLEVEL_CRON_SUPPORT

/* Cron configuration:
 * Fields: Min Hour Day Month Dow
 * Values:
 * * -1    for every value in this field ( like * in a normal cron )
 * * >0    for exactly this value in this field
 * * < -1  step value, e.g. -2: every second minute
 */
#define USE_UTC 1
#define USE_LOCAL 0

const struct cron_static_event_t events[] PROGMEM = {
#ifdef MCUF_CLOCK_SUPPORT
  {{{{-1, -1, -1, -1}}, -1}, mcuf_clock, USE_LOCAL},    /* every minute  */
#endif /* MCUF_CLOCK_SUPPORT */

#ifdef MCUF_MODUL_CRON_SUPPORT
  {{{{-1, -1, -1, -1}}, -1}, mcuf_modul, USE_LOCAL},    /* every minute  */
#endif // MCUF_MODUL_CRON_SUPPORT

#ifdef NETSTAT_SUPPORT
  {{{{-5, -1, -1, -1}}, -1}, (cron_static_handler_t) netstat_send, USE_LOCAL},  /* every 5 minutes  */
#endif // NETSTAT_SUPPORT

#ifdef TANKLEVEL_CRON_SUPPORT
  {{{{0,  0, -1, -1}}, -1}, (cron_static_handler_t)tanklevel_start, USE_LOCAL}, /* 0 a clock  */
  {{{{0, 12, -1, -1}}, -1}, (cron_static_handler_t)tanklevel_start, USE_LOCAL}, /* 12 a clock  */
#endif // TANKLEVEL_CRON_SUPPORT

  /* This is only the end of table marker */
  {{{{-1, -1, -1, -1}}, -1}, NULL, 0},
};

void
cron_static_periodic(void)
{
  /* convert time to something useful */
  clock_datetime_t d, ld;
  uint32_t timestamp = clock_get_time();

  /* fix last_check */
  if (timestamp < last_check)
  {
    clock_datetime(&d, timestamp);
    last_check = timestamp - d.sec;
    return;
  }

  /* Only check the tasks every minute */
  if ((timestamp - last_check) < 60)
    return;

  clock_datetime(&d, timestamp);
  clock_localtime(&ld, timestamp);

  struct cron_static_event_t event;

  /* check every event for a match */
  for (uint8_t i = 0;; i++)
  {
    memcpy_P(&event, &events[i], sizeof(struct cron_static_event_t));

    /* end of task list reached */
    if (event.handler == NULL)
      break;

    /* if it matches, execute the handler function */
    if (cron_check_event(&event.cond, event.use_utc, &d, &ld))
    {
      event.handler();
    }
  }

  /* save the actual timestamp */
  last_check = timestamp - d.sec;
}

/*
  -- Ethersex META --
  header(services/cron/cron_static.h)
  timer(50, cron_static_periodic())
*/
