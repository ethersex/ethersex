/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "bulbdial.h"
#include "services/clock/clock.h"
#include "core/portio/user_config.h"

#include <util/delay.h> 

void set(uint16_t value){
  switch (value) {
  	case 0x0001: PIN_SET(BULBDIAL_ONE); break;
  	case 0x0002: PIN_SET(BULBDIAL_TWO); break;
  	case 0x0004: PIN_SET(BULBDIAL_THREE); break;
  	case 0x0008: PIN_SET(BULBDIAL_FOUR); break;
  	case 0x0010: PIN_SET(BULBDIAL_FIVE); break;
  	case 0x0020: PIN_SET(BULBDIAL_SIX); break;
  	case 0x0040: PIN_SET(BULBDIAL_SEVEN); break;
  	case 0x0080: PIN_SET(BULBDIAL_EIGHT); break;
  	case 0x0100: PIN_SET(BULBDIAL_NINE); break;
  	case 0x0200: PIN_SET(BULBDIAL_TEN); break;
  	case 0x0400: PIN_SET(BULBDIAL_ELEVEN); break;
  	case 0x0800: PIN_SET(BULBDIAL_TWELVE); break;
  }
}

void clear(){
  PIN_CLEAR(BULBDIAL_ONE);
  PIN_CLEAR(BULBDIAL_TWO);
  PIN_CLEAR(BULBDIAL_THREE);
  PIN_CLEAR(BULBDIAL_FOUR);
  PIN_CLEAR(BULBDIAL_FIVE);
  PIN_CLEAR(BULBDIAL_SIX);
  PIN_CLEAR(BULBDIAL_SEVEN);
  PIN_CLEAR(BULBDIAL_EIGHT);
  PIN_CLEAR(BULBDIAL_NINE);
  PIN_CLEAR(BULBDIAL_TEN);
  PIN_CLEAR(BULBDIAL_ELEVEN);
  PIN_CLEAR(BULBDIAL_TWELVE);
}

void
bulbdial_clock(uint8_t delay){
  clock_datetime_t date;
  clock_current_localtime(&date);
  clear();
  set(0x0001 << (date.sec/5));
  PIN_SET(BULBDIAL_MINUTES);
  for (uint8_t i = delay; i; i--)
    _delay_ms(1);
  PIN_CLEAR(BULBDIAL_MINUTES);
  clear();
  set(0x0001 << (date.min/5));
  PIN_SET(BULBDIAL_HOURS);
  for (uint8_t i = delay; i; i--)
    _delay_ms(1);
  PIN_CLEAR(BULBDIAL_HOURS);
}


void
bulbdial_periodic(void)
{

  bulbdial_clock(4);

}

/*
  -- Ethersex META --
  header(services/bulbdial/bulbdial.h)
  mainloop(bulbdial_periodic)
*/
