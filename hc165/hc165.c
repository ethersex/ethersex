/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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
 }}} */

#include <avr/io.h>
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../portio.h"
#include "../config.h"
#include "hc165.h"

#ifdef HC165_SUPPORT

void
hc165_init(void)
{
  /* Initialize the pins */
  HC165_DDR |= _BV(HC165_CLOCK_PIN) |  _BV(HC165_LOAD_PIN);
  HC165_DDR &= ~_BV(HC165_DATA_PIN);
  
  HC165_PORT &= ~_BV(HC165_CLOCK_PIN);
  HC165_PORT |= _BV(HC165_LOAD_PIN) | _BV(HC165_DATA_PIN);
} 

uint8_t 
hc165_read_pin(uint8_t port) 
{
  uint8_t result;
  /* Skip the Hardware ports */
  port -= IO_HARD_PORTS;

  /* Load Parallel */
  HC165_PORT &= ~_BV(HC165_LOAD_PIN);
  HC165_PORT |= _BV(HC165_LOAD_PIN);

  do {
    uint8_t i = 0;
    while (i < 8) {
      result <<= 1;
#if HC165_INVERSE_OUTPUT
      if (!(HC165_PIN & _BV(HC165_DATA_PIN))) 
#else
      if (HC165_PIN & _BV(HC165_DATA_PIN))
#endif
        result |= 1;
      HC165_PORT |= _BV(HC165_CLOCK_PIN);
      HC165_PORT &= ~_BV(HC165_CLOCK_PIN);
      i++;
    }
  } while (port--);
  return result;
}

#endif

