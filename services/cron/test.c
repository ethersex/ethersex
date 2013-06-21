/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include "test.h"
#include "cron.h"
#include "config.h"
#include "core/debug.h"

void
test(void *data)
{
#ifdef DEBUG
  debug_printf("cron test event\n");
#endif
}

void
addcrontest()
{
  /* add jump_to_function crons */
  cron_jobinsert_callback(-1, -2, -1, -1, -1, INFINIT_RUNNING, CRON_APPEND, test, 0, NULL);     /* when hour % 2 == 0 */
  cron_jobinsert_callback(51, -1, -1, -1, -1, INFINIT_RUNNING, CRON_APPEND, test, 0, NULL);     /* when minute is 51 */
  /* add ecmd cron */
  // example ecmd for stella to set channel 0 to 120
  cron_jobinsert_ecmd(-1, -1, -1, -1, -1, INFINIT_RUNNING, CRON_APPEND, "channel 0 120 f");     /* when minute % 2 == 0 */
}
