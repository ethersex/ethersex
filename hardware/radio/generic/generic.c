/*
* Copyright (c) 2014 Erik Kunze <ethersex@erik-kunze.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
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

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "config.h"
#include "generic.h"


void
generic_ask_trigger(uint8_t level, uint16_t us)
{
  if (level)
  {
    PIN_SET(GENERIC_ASK_TX);
  }
  else
  {
    PIN_CLEAR(GENERIC_ASK_TX);
  }
  for (; us > 0; us--)
    _delay_us(1);
}

