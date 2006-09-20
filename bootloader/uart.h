/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _UART_H
#define _UART_H

/* define uart baud rate and mode (8N1) */
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
/* {{{ */
/* in atmega8, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC (_BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1))

/* }}} */
#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega644__)
/* {{{ */
/* in atmega88, this isn't needed any more */
#define UART_UCSRC (_BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0))

/* }}} */
#else
#error this cpu isn't supported by uart.h yet!
#endif

/* define UBRR value (see datasheet */
#if (F_CPU == 16000000)
#define UART_UBRR 8 /* 16mhz, 115200 */
/* #define UART_UBRR 51 */ /* 16mhz, 19200 */
#elif (F_CPU == 20000000)
#define UART_UBRR 10 /* 20mhz, 115200 */
#elif (F_CPU == 8000000)
/* #define UART_UBRR 25 */ /* 8mhz, 19200 */
/* #define UART_UBRR 3 */ /* 8mhz, 115200 */
#else
#error this cpu frequency is not supported by uart.h yet!
#error (see datasheet for UBRR value at 115200 baud)
#endif

#endif /* _UART_H */
