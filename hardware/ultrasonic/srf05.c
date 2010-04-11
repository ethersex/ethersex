/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ultrasonic.h"

int16_t
srf05_get(void)
{
  ULTRASONICDEBUG ("get\n");
// disable interrupts
  uint8_t sreg = SREG; cli();
  DDR_CONFIG_OUT(SRF05); //  switch to output
  PIN_SET(SRF05);  // start init sonic ping
  _delay_us(1);
  PIN_CLEAR(SRF05);
  _delay_us(3);
  PIN_SET(SRF05); // ping!
  _delay_us(15);
  PIN_CLEAR(SRF05);
  DDR_CONFIG_IN(SRF05); //  switch to input

// min 700us wait until pin high 
  uint16_t i=0;
  for (i=0; i < 1000; i++){
    if (PIN_HIGH(SRF05)) break;
    _delay_us(1);
  }
//  loop delay max 30ms
  for (i=0; i < 30000; i++){
    if (!PIN_HIGH(SRF05)) break;
    _delay_us(1);
  }
  SREG = sreg;			/* Possibly re-enable interrupts. */

  // now check value, might be not usefull...
  if (i == 30000) {
    ULTRASONICDEBUG ("overrun\n");
	return -1;
  }
  ULTRASONICDEBUG ("SRF05: %i\n",i);

  return i;
}

