/*
 * Copyright(C) 2006 Christian Dietrich <stettberger@dokucode.de>
 * Copyright(C) 2006 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2007 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright(C) 2008 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2009 David Gräff <david.graeff@web.de>
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

#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "stella.h"
#include "core/debug.h"
#define ACCESS_IO(x) (*(volatile uint8_t *)(x))

stella_timetable_entry_s *current = 0;

/* Use port mask to switch pins on if timetable says so and
 * set the next trigger point in time for the compare interrupt.
 * */
#ifdef STELLA_LOW_PRIORITY
// other interrupts can interrupt this ISR
ISR(STELLA_TC_VECTOR_COMPARE, ISR_NOBLOCK)
{
  // disable the interrupt we are in
  // makes sure we don't interrupt ourselves
  STELLA_TC_INT_COMPARE_OFF;
#else
ISR(STELLA_TC_VECTOR_COMPARE)
{
#endif
  // Activate all timetable entries for this timepoint
  // We may have more than one for a certain timepoint, if ports in the timetable entries differ
  while (current)
  {
    ACCESS_IO(current->port.port) |= current->port.mask;
    current = current->next;

    if (current && STELLA_TC_COMPARE_REG != current->value)
    {
      STELLA_TC_COMPARE_REG = current->value;
      break;
    }
  }

#ifdef STELLA_LOW_PRIORITY
  // enable our interrupt again
  STELLA_TC_INT_COMPARE_ON;
#endif
}

/* If channel values have been updated (update_table is set) update all i_* variables.
 * Start the next pwm round. */

#ifdef STELLA_LOW_PRIORITY
// other interrupts can interrupt this ISR
ISR(STELLA_TC_VECTOR_OVERFLOW, ISR_NOBLOCK)
{
  // disable the interrupt we are in
  // makes sure we don't interrupt ourselves
  STELLA_TC_INT_OVERFLOW_OFF;
#else
ISR(STELLA_TC_VECTOR_OVERFLOW)
{
#endif
  /* if new values are available, work with them */
  if (stella_sync == NEW_VALUES)
  {
    // swap pointer
    struct stella_timetable_struct *temp = int_table;
    int_table = cal_table;
    cal_table = temp;

    // reset update flag
    stella_sync = NOTHING_NEW;
  }

  /* count down the fade_timer counter. If zero, we process
   * stella_process from ethersex main */
  if (stella_fade_counter)
    stella_fade_counter--;

  /* Start the next pwm round */
  current = int_table->head;

  /* Leave all non-stella-pins the same and activate pins used by the first timetable entry. */
  for (uint8_t i = 0; i < STELLA_PORT_COUNT; ++i)
    ACCESS_IO(int_table->port[i].port) =
      (ACCESS_IO(int_table->port[i].port) & ~(uint8_t) stella_portmask[i]) |
      int_table->port[i].mask;

  if (current)
    STELLA_TC_COMPARE_REG = current->value;

#ifdef STELLA_LOW_PRIORITY
  // enable our interrupt again
  STELLA_TC_INT_OVERFLOW_ON;
#endif
}
