/*
 * scheduler/dynamic.c
 *
 * Copyright (c) 2013-2015 by Michael Brakemeier <michael@brakemeier.de>
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

#include "config.h"

#include "dynamic.h"

/**
 * Add a dynamic timer.
 *
 * Timers can be deleted by means of scheduler_delete_timer().
 * Note: A timer *MUST NOT* delete itself!
 *
 * @param func timer function to add.
 * @param interval interval at which the timer func is called.
 *
 * @return a handle with a positive value on success, a negative value otherwise.
 */
int
scheduler_add_timer(timer_t func, uint16_t interval)
{
  for(uint8_t index = 0; index < scheduler_timer_max; index++)
  {
    /* find free entry */
    if (scheduler_timers[index].state == TIMER_DELETED)
    {
      /* and add timer */
      scheduler_timers[index].timer = func;
      scheduler_timers[index].delay = interval;
      scheduler_timers[index].interval = interval;
      scheduler_timers[index].state = (TIMER_DYNAMIC | TIMER_RUNNABLE);

      return index;
    }
  }

  return SCHEDULER_AGAIN;
}

/**
 * Delete a dynamic timer.
 *
 * Note: A timer *MUST NOT* delete itself!
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int
scheduler_delete_timer(int which)
{
  if ((which < scheduler_timer_max) &&
      ((scheduler_timers[which].state & TIMER_DYNAMIC) == TIMER_DYNAMIC))
  {
    scheduler_timers[which].delay = SCHEDULER_INTERVAL_MAX;
    scheduler_timers[which].state = TIMER_DELETED;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}
