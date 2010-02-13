/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 3)
 * as published by the Free Software Foundation.
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

#include <avr/io.h>
#include "config.h"

#include "tlc5620.h"

void
tlc5620_set_channel(uint8_t channel, uint8_t value)
{
  unsigned short i = 8;
  PIN_SET(TLC5620_LOAD);
  /* Address */
  PIN_CLEAR(TLC5620_DATA);
  if (channel & 0x02)
    PIN_SET(TLC5620_DATA);
  PIN_TOGGLE(TLC5620_CLOCK);
  PIN_TOGGLE(TLC5620_CLOCK);
  
  PIN_CLEAR(TLC5620_DATA);
  if (channel & 0x01)
    PIN_SET(TLC5620_DATA);
  PIN_TOGGLE(TLC5620_CLOCK);
  PIN_TOGGLE(TLC5620_CLOCK);

  /* RNG */
  PIN_SET(TLC5620_DATA);
  PIN_TOGGLE(TLC5620_CLOCK);
  PIN_TOGGLE(TLC5620_CLOCK);

  /* Value */
  while (i) {
    PIN_CLEAR(TLC5620_DATA);
    if (value & (1 << i)) 
      PIN_SET(TLC5620_DATA);
    PIN_TOGGLE(TLC5620_CLOCK);
    PIN_TOGGLE(TLC5620_CLOCK);
    i--;
  }
  PIN_CLEAR(TLC5620_LOAD);
  PIN_SET(TLC5620_LOAD);
}
