/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../bit-macros.h"
#include "../config.h"
#include "../syslog/syslog.h"
#include "ps2.h"

#ifdef PS2_SUPPORT
volatile uint8_t bitcount;
volatile uint8_t data;
volatile uint8_t is_up;
volatile uint8_t parity;
volatile uint8_t timeout;

void
ps2_init(void) 
{
  bitcount = 11;
  parity = 1;

  PCICR |= _BV(PS2_PCIE);
  PS2_PCMSK |= _BV(PS2_CLOCK_PIN);

  PS2_DDR &= ~(_BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN));
  PS2_PORT &= ~(_BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN));
}

void
ps2_periodic(void)
{
  if(timeout--)
    if(timeout == 0) {
      /* Reset the reciever */
      parity = 1;
      bitcount = 11;
    }
}

SIGNAL(PS2_INTERRUPT) 
{
  if (!(PS2_PIN & _BV(PS2_CLOCK_PIN))) {
    /* Start the timeout to 20ms - 40ms */
    if (bitcount == 1) 
      timeout = 2;

    if (bitcount < 11 && bitcount > 2) {
      data >>= 1;
      if ( PINA & _BV(PS2_DATA_PIN)) {
        data |= 0x80;
        parity ^= 1;
      }
    } else if (bitcount == 2) {
      if (((PS2_PIN & _BV(PS2_DATA_PIN)) ^ (parity << PS2_DATA_PIN)) != 0) {
        data = 0;
      }
    }

    if (--bitcount == 0) {
      if (is_up) {
        syslog_sendf("Key: %x %c", data, data);
        is_up = 0;
      } else if (data == 0xF0 && ! is_up) 
        is_up = 1;
      parity = 1;
      bitcount = 11;
    }
  }
}

#endif
