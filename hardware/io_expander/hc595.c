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

#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/portio/portio.h"

static uint8_t hc595_cache[HC595_REGISTERS];
void hc595_update(void);

void
hc595_init(void)
{
  uint8_t i;
  for (i = 0; i < HC595_REGISTERS; i++)
    hc595_cache[i] = 0;

  DDR_CONFIG_OUT(HC595_DATA);
  DDR_CONFIG_OUT(HC595_CLOCK);
  DDR_CONFIG_OUT(HC595_STORE);
  PIN_SET(HC595_STORE);

  /* FIXME: maybe must set the output enable pin */

  hc595_update();
} 

uint8_t 
hc595_write_port(uint8_t port, uint8_t data) {
  hc595_cache[port - IO_HARD_PORTS] = data;
  hc595_update();
  return 0;
}
uint8_t 
hc595_read_port(uint8_t port) 
{
  return hc595_cache[port - IO_HARD_PORTS];
}

void
hc595_update(void) 
{
  uint8_t i, x;
  PIN_CLEAR(HC595_CLOCK);
  PIN_CLEAR(HC595_STORE);

  for ( i = HC595_REGISTERS; i;) {
    i --;
    for ( x = 8; x;) {
      PIN_CLEAR(HC595_DATA);
      if (hc595_cache[i] & _BV(--x))
        PIN_SET(HC595_DATA);
      
      /* Pulse the hc595 shift clock */
      PIN_SET(HC595_CLOCK);
      PIN_CLEAR(HC595_CLOCK);
    }
  }
  /* Pulse the hc595 store clock to load the shifted bits */
  PIN_SET(HC595_STORE);
  PIN_CLEAR(HC595_STORE);
}

/*
  -- Ethersex META --
  header(hardware/io_expander/hc595.h)
  init(hc595_init)
*/

