/*
 * scheduler/oneshot.c
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

#include "oneshot.h"

/**
 * Add a dynamic one-shot timer.
 */
int
scheduler_add_oneshot_timer(timer_t func, uint16_t delay)
{
  for(uint8_t i = 0; i < scheduler_timer_max; i++)
  {
    /* find free entry */
    if ((scheduler_timers[i].state == TIMER_DELETED)
    {
      // and add timer
      scheduler_timers[i].timer = func;
      scheduler_timers[i].delay = delay;
      scheduler_timers[i].interval = SCHEDULER_INTERVAL_MAX;
      scheduler_timers[i].state = (TIMER_ONESHOT | TIMER_DYNAMIC | TIMER_RUNNABLE);

      return SCHEDULER_OK;
    }
  }

  return SCHEDULER_AGAIN;
}
