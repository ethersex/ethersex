/*
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
 */

#include <avr/io.h>
#include "core/portio/portio.h"
#include "config.h"
#include "hc165.h"

void
hc165_init(void)
{
  /* Initialize the pins */
  DDR_CONFIG_OUT(HC165_CLOCK);
  DDR_CONFIG_OUT(HC165_LOAD);
  DDR_CONFIG_IN(HC165_DATA);
  
  PIN_CLEAR(HC165_CLOCK);
  PIN_SET(HC165_LOAD);
  PIN_SET(HC165_DATA);
} 

uint8_t 
hc165_read_pin(uint8_t port) 
{
  uint8_t result;
  /* Skip the Hardware ports, because the portio pin 4 is the hc165 pin 0 */
  port -= IO_HARD_PORTS;

  /* Load Parallel */
  PIN_CLEAR(HC165_LOAD);
  PIN_SET(HC165_LOAD);

  do {
    uint8_t i = 0;
    while (i < 8) {
      result <<= 1;
#if HC165_INVERSE_OUTPUT
      if (!PIN_HIGH(HC165_DATA))
#else
      if (PIN_HIGH(HC165_DATA))
#endif
        result |= 1;
      PIN_SET(HC165_CLOCK);
      PIN_CLEAR(HC165_CLOCK);
      i++;
    }
  } while (port--);
  return result;
}

/*
  -- Ethersex META --
  header(hardware/io_expander/hc165.h)
  init(hc165_init)
*/
