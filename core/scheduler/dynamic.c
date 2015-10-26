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

#include <util/atomic.h>

/**
 * Add a dynamic timer.
 */
static int8_t add_timer(timer_t func, uint16_t delay, uint16_t interval, uint8_t flags);


/**
 * Add a dynamic timer.
 */
static int8_t
add_timer(timer_t func, uint16_t delay, uint16_t interval, uint8_t flags)
{
  for(uint8_t index = 0; index < scheduler_dynamic_timer_max; index++)
  {
    /* find free entry */
    if (scheduler_dynamic_timers[index].state == TIMER_DELETED)
    {
      /* and add timer */
      scheduler_dynamic_timers[index].timer = func;
      scheduler_dynamic_timers[index].delay = delay;
      scheduler_dynamic_timers[index].interval = interval;
      scheduler_dynamic_timers[index].state = flags;

      return (int8_t)index;
    }
  }

  return SCHEDULER_AGAIN;
}


/**
 * Add a dynamic timer.
 *
 * Timers can be deleted by means of scheduler_delete_timer().
 *
 * @param func timer function to add.
 * @param interval interval at which the timer func is called.
 * @param suspended if true, the timer will be initialized in suspended state.
 *
 * @return a handle with a positive value on success, a negative value otherwise.
 */
int8_t
scheduler_add_timer(timer_t func, uint16_t interval, bool suspended)
{
  return add_timer(func, interval, interval,
                   suspended ? TIMER_SUSPENDED : TIMER_RUNNABLE);
}


/**
 * Add a dynamic one-shot timer.
 *
 * One-shot timers are a cheap method to add a fully e6-"multitasking"-
 * compatible-non-blocking delay.
 *
 * @param func one-shot timer function to add.
 * @param delay delay until the timer is invoked exactly once.
 *
 * @return a handle with a positive value on success, a negative value otherwise.
 */
int8_t
scheduler_add_oneshot_timer(timer_t func, uint16_t delay)
{
  return add_timer(func, delay, 0, (TIMER_ONESHOT | TIMER_RUNNABLE));
}


/**
 * Delete a dynamic timer.
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int8_t
scheduler_delete_timer(int8_t which)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    scheduler_dynamic_timers[which].delay = SCHEDULER_INTERVAL_MAX;
    scheduler_dynamic_timers[which].state = TIMER_DELETED;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}


/**
 * Suspend a timer.
 *
 * Suspend/stop a timer from normal operation. The timer will not expire
 * and the timer function will not be called until the timer is either
 * resumed or reset.
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int8_t
scheduler_suspend_timer(int8_t which)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    scheduler_dynamic_timers[which].state &= (uint8_t)~TIMER_RUNNABLE;
    scheduler_dynamic_timers[which].state |= TIMER_SUSPENDED;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}


/**
 * Resume a suspended timer.
 *
 * Resume/wake-up a suspended timer. The timer's delay counter is
 * *NOT* reset, the timer will resume where it has been suspended.
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int8_t
scheduler_resume_timer(int8_t which)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    scheduler_dynamic_timers[which].state &= (uint8_t)~TIMER_SUSPENDED;
    scheduler_dynamic_timers[which].state |= TIMER_RUNNABLE;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}


/**
 * Reset/Restart a timer.
 *
 * Reset/restart a timer by setting the delay counter to the initial
 * interval. If the timer is currently suspended it will be resumed.
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int8_t
scheduler_reset_timer(int8_t which)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    // reset delay
    scheduler_dynamic_timers[which].delay = scheduler_dynamic_timers[which].interval;

    // and (always) reset SUSPENDED flags, simply ignore if it's not set
    scheduler_dynamic_timers[which].state &= (uint8_t)~TIMER_SUSPENDED;
    scheduler_dynamic_timers[which].state |= TIMER_RUNNABLE;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}

/**
 * Get a timer's current interval setting.
 *
 * @param which the handle returned by scheduler_add_timer().
 *
 * @return the timer's current interval, or zero if the handle passed in is invalid
 */
uint16_t scheduler_get_timer_interval(int8_t which)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    return scheduler_dynamic_timers[which].interval;
  }

  return 0;
}

/**
 * Set a timer's current interval.
 *
 * @param which the handle returned by scheduler_add_timer().
 * @param new_interval the timer's new_interval.
 *
 * @return SCHEDULER_OK (0) on success, <0 otherwise.
 */
int8_t scheduler_set_timer_interval(int8_t which, uint16_t new_interval)
{
  if ((which >= 0) && (which < scheduler_dynamic_timer_max))
  {
    scheduler_dynamic_timers[which].interval = new_interval;

    return SCHEDULER_OK;
  }

  return SCHEDULER_INVAL;
}

