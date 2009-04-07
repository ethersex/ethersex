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
#include <avr/interrupt.h>
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "config.h"
#include "protocols/zbus/zbus.h"
#include "protocols/modbus/modbus.h"
#include "protocols/yport/yport.h"
#include "core/usart.h"

#ifndef TEENSY_SUPPORT
/* Sets the ubrr register according to baudrate
 * The baudrate had to be baudrate/100 */
uint16_t
usart_baudrate(uint16_t baudrate) {
  uint16_t ubrr;

  switch(baudrate) {
/* We use here precalucated values, because the floating point aritmetic would
 * be too expensive */
#if F_CPU == 20000000UL
  case 24:
    ubrr = 520;
    break;
  case 144:
    ubrr = 86;
    break;
  case 384:
    ubrr = 32;
    break;
  case 576:
    ubrr = 21;
    break;
  case 1152:
    ubrr = 10;
    break;
#endif
  default:
    ubrr = (F_CPU/1600) / baudrate - 1;
  }
  return ubrr;
}

#endif
