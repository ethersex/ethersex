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

#include <stdlib.h>

#include "config.h"

#include "protocols/ecmd/ecmd-base.h"

#include "core/periodic.h"
#include "core/debug.h"

#ifdef DEBUG_PERIODIC
#define PERIODICDEBUG(a...)   debug_printf("periodic: " a)
extern volatile uint16_t pdebug_milliticks;
extern volatile uint16_t pdebug_milliticks_max;
extern volatile uint16_t pdebug_milliticks_miss;
#else
#define PERIODICDEBUG(a...)
#endif /* DEBUG_PERIODIC */

#ifdef PERIODIC_ADJUST_SUPPORT
int16_t
parse_cmd_periodic_adjust(char *cmd, char *output, uint16_t len)
{
  uint16_t newtop = 0;

  while (*cmd == ' ')
    cmd++;

  if (*cmd == 0)
  {
    newtop = PERIODIC_COUNTER_COMPARE;
  }
  else
  {
    int16_t offset = atoi(cmd);

    newtop = periodic_adjust_set_offset(offset);

    PERIODICDEBUG("param offset was %d, new TOP %u\n", offset, newtop);
  }

  if (newtop == 0)
    return ECMD_ERR_PARSE_ERROR;

  len = snprintf(output, len, "%u", newtop);

  return ECMD_FINAL(len);
}
#endif

#ifdef DEBUG_PERIODIC_ECMD_SUPPORT
enum
{
  PRINT_TIMER_USED = 0,
  PRINT_MILLITICKS,
  PRINT_MILLITICKS_MISS,
  LAST
};

int16_t
parse_cmd_periodic_stats(char *cmd, char *output, uint16_t len)
{
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    /* indicator flag, first call: 0 */
    cmd[0] = ECMD_STATE_MAGIC;  /* continuing call: 23 */
    cmd[1] = 0;                 /* count states / output */
  }

  /* simple state machine to iterate over the lines of output */
  switch (cmd[1])
  {
    case PRINT_TIMER_USED:
      len =
        snprintf_P(output, len, PSTR("Use T/C %d, PRESCALER: %u, TOP: %u"),
                   CONF_PERIODIC_USE_TIMER,
                   (unsigned int) (PERIODIC_PRESCALER),
#ifndef PERIODIC_ADJUST_SUPPORT
                   (unsigned int) (PERIODIC_TOP));
#else
                   (unsigned int) (PERIODIC_COUNTER_COMPARE));
#endif
      break;

    case PRINT_MILLITICKS:
      len = snprintf_P(output, len,
                       PSTR("milliticks 1/s: %u, current: %u, max: %u"),
                       CONF_MTICKS_PER_SEC, pdebug_milliticks,
                       pdebug_milliticks_max);
      break;

    case PRINT_MILLITICKS_MISS:
      len =
        snprintf_P(output, len, PSTR("milliticks missed: %u"),
                   pdebug_milliticks_miss);
      break;
  }

  cmd[1]++;
  if (cmd[1] == LAST)
  {
    return ECMD_FINAL(len);
  }

  return ECMD_AGAIN(len);
}

int16_t
parse_cmd_periodic_reset(char *cmd, char *output, uint16_t len)
{
  pdebug_milliticks_max = 0;
  pdebug_milliticks_miss = 0;

  len = snprintf_P(output, len, PSTR("OK"));

  return ECMD_FINAL(len);
}
#endif /* DEBUG_PERIODIC_ECMD_SUPPORT */

/*
 -- Ethersex META --
 ecmd_ifdef(PERIODIC_ADJUST_SUPPORT)
 block(Miscellaneous)
 ecmd_feature(periodic_adjust, "periodic adjust", OFFSET, Adjust periodic timers TOP value by adding OFFSET ticks.)
 ecmd_endif()
 ecmd_ifdef(DEBUG_PERIODIC_ECMD_SUPPORT)
 block(Miscellaneous)
 ecmd_feature(periodic_stats, "periodic stats",, Print debug statistics for periodic module.)
 ecmd_feature(periodic_reset, "periodic reset",, Reset debug statistics for periodic module.)
 ecmd_endif()
 */
