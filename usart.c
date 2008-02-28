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
#include <avr/interrupt.h>
#include "eeprom.h"
#include "bit-macros.h"
#include "config.h"
#include "zbus/zbus.h"
#include "modbus/modbus.h"
#include "yport/yport.h"
#include "usart.h"

#ifdef USART_SUPPORT


void
usart_init(void) 
{
    /* The ATmega644 datasheet suggests to clear the global
       interrupt flags on initialization ... */
    uint8_t sreg = SREG; cli();

#ifndef TEENSY_SUPPORT
    usart_baudrate(eeprom_read_word(&(((struct eeprom_config_ext_t *)
                                       EEPROM_CONFIG_EXT)->usart_baudrate)));
#else

/* the used ubrr value */
#if defined(YPORT_SUPPORT)
# define UART_UBBR YPORT_UART_UBRR
#elif defined(ZBUS_SUPPORT)
# define UART_UBRR ZBUS_UART_UBRR
#elif defined(MODBUS_SUPPORT)
# define UART_UBRR MODBUS_UART_UBRR
#endif

    /* set baud rate */
    _UBRRH_UART0 = HI8(UART_UBRR);
    _UBRRL_UART0 = LO8(UART_UBRR);
#endif

#ifdef URSEL
    /* set mode: 8 bits, 1 stop, no parity, asynchronous usart
       and Set URSEL so we write UCSRC and not UBRRH */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01) | _BV(URSEL);
#else
    /* set mode: 8 bits, 1 stop, no parity, asynchronous usart */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);
#endif

    /* Enable the RX interrupt and receiver and transmitter */
    _UCSRB_UART0 |= _BV(_TXEN_UART0) | _BV(_RXEN_UART0) | _BV(_RXCIE_UART0);

    /* Go! */
    SREG = sreg;
}

#ifndef TEENSY_SUPPORT
/* Sets the ubrr register according to baudrate 
 * The baudrate had to be baudrate/100 */
void 
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
  /* set baud rate */
  _UBRRH_UART0 = HI8(ubrr);
  _UBRRL_UART0 = LO8(ubrr);
}

#endif
#endif
