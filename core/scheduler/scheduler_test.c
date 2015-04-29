/*
 * scheduler/scheduler_test.c
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

#include "scheduler.h"

#include "scheduler_test.h"

#include <services/clock/clock.h>

void scheduler_test_periodic_t50(void)
{
  SCHEDULERDEBUG("test timer(50, ...) - %lu\n", clock_get_time());
}

void scheduler_test_periodic_mt1000(void)
{
  SCHEDULERDEBUG("test millitimer(1000, ...) - %lu\n", clock_get_time());
}
/*
  -- Ethersex META --
  header(core/scheduler/scheduler_test.h)
  ifdef(`conf_DEBUG_SCHEDULER',`timer(50, scheduler_test_periodic_t50)')
  ifdef(`conf_DEBUG_SCHEDULER',`millitimer(1000, scheduler_test_periodic_mt1000)')
*/
