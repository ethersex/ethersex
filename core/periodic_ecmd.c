/*
 *
 * Copyright (c) 2014 by Michael Brakemeier <michael@brakemeier.de>
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

#include "config.h"

#include "protocols/ecmd/ecmd-base.h"

#include "core/periodic.h"
#include "core/debug.h"

#ifdef DEBUG_PERIODIC_ECMD_SUPPORT
extern volatile uint16_t milliticks_min;
extern volatile uint16_t milliticks_max;
extern volatile uint16_t milliticks_miss;

int16_t
parse_cmd_periodic_debug_stats(char *cmd, char *output, uint16_t len)
{
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    /* indicator flag, first call: 0 */
    cmd[0] = ECMD_STATE_MAGIC;  /* continuing call: 23 */
    cmd[1] = 0;                 /* count states / output */
  }

  /* simple state machine to iterate over the lines of output */
  enum
  {
    PRINT_TIMER_USED = 0,
    PRINT_COUNTS,
    PRINT_MILLITICKS,
    PRINT_MILLITICKS_MISS,
    LAST
  };

  switch (cmd[1])
  {
    case PRINT_TIMER_USED:
      len =
        snprintf_P(output, len, PSTR("Use Timer/Counter %d, Prescaler: %u"),
                   CONF_PERIODIC_USE_TIMER,
                   (unsigned int) (CLOCK_PRESCALER));
      break;

    case PRINT_COUNTS:
      len =
        snprintf_P(output, len, PSTR("Timer Counts/Tick: %u, Counts/Hz: %u"),
                   (unsigned int) CLOCK_MILLITICKS,
                   (unsigned int) CLOCK_TICKS);
      break;

    case PRINT_MILLITICKS:
      len = snprintf_P(output, len,
                   PSTR("Ticks 1/s: %u, Tick current: %u, min: %u, max: %u"),
                   CLOCKS_PER_SEC, milliticks, milliticks_min, milliticks_max);
      break;

    case PRINT_MILLITICKS_MISS:
      len = snprintf_P(output, len, PSTR("Ticks missed: %u"), milliticks_miss);
      break;
  }

  cmd[1]++;
  if (cmd[1] == LAST)
  {
    return ECMD_FINAL(len);
  }

  return ECMD_AGAIN(len);
}
#endif /* DEBUG_PERIODIC_ECMD_SUPPORT */


/*
 -- Ethersex META --
 ecmd_ifdef(DEBUG_PERIODIC_ECMD_SUPPORT)
 block(Miscelleanous)
 ecmd_feature(periodic_debug_stats, "periodicstats",, Print debug statistics for periodic module)
 ecmd_endif()
 */
