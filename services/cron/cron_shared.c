/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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

#include "cron_shared.h"

uint8_t
cron_check_event(cron_conditions_t * cond, uint8_t use_utc,
                 clock_datetime_t * d, clock_datetime_t * ld)
{
  clock_datetime_t *cd = (use_utc) ? d : ld;

  /* check time */
  for (uint8_t f = 0; f <= 3; f++)
  {

    /* if this field has a wildcard, just go on checking */
    if (cond->fields[f] == -1)
      continue;

    /* IF This field has an absolute value, check this value, if it does
     * not match, this event does not match */
    if (cond->fields[f] >= 0 && cond->fields[f] != cd->cron_fields[f])
      return 0;

    /* if this field has a step value, extract value and check */
    if (cond->fields[f] < 0)
    {
      uint8_t step = -(cond->fields[f]);

      /* if this is not within the steps, this event does not match */
      if ((d->cron_fields[f] % step) != 0)
        return 0;

    }

  }

  /* check weekdays */
  if ((cond->daysofweek & _BV(cd->dow)) == 0)
    return 0;

  /* if all fields match, this event matches */
  return 1;
}
