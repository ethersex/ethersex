/*
 * scheduler/scheduler.c
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

#include "scheduler.h"

/**
 * Initialize the scheduler.
 */
void
scheduler_init(void)
{
  // TODO? add run-time optimizations to the array here if req'ed

  return;
}

/**
 * The scheduler loop.
 */
void
scheduler_dispatch_timer(void)
{
  for(uint8_t i = 0; i < scheduler_timer_max; i++)
  {
    // check for a valid timer ready to run
    if(((scheduler_timers[i].state & TIMER_RUNNABLE) == TIMER_RUNNABLE)
        && (scheduler_timers[i].delay == 0))
    {
      // call timer
      scheduler_timers[i].state &= (uint8_t)~TIMER_RUNNABLE;
      scheduler_timers[i].state |= TIMER_RUNNING;

      (*scheduler_timers[i].timer)();

#ifdef SCHEDULER_ONESHOT_SUPPORT
      if ((scheduler_timers[i].state & TIMER_ONESHOT) != TIMER_ONESHOT)
#endif
      {
        // reset delay
        scheduler_timers[i].delay = scheduler_timers[i].interval;

        // set back to runnable
        scheduler_timers[i].state &= (uint8_t)~TIMER_RUNNING;
        scheduler_timers[i].state |= TIMER_RUNNABLE;
      }
#ifdef SCHEDULER_ONESHOT_SUPPORT
      else
      {
        /* auto-delete one-shot timer */
        scheduler_timers[i].delay = 0;
        scheduler_timers[i].state = TIMER_DELETED;
      }
#endif
    }
  }

  return;
}

/*
  -- Ethersex META --
  header(core/scheduler/scheduler.h)
  initearly(scheduler_init)
  periodic_milliticks_header(core/scheduler/scheduler.h)
  periodic_milliticks_isr(scheduler_tick())
*/
