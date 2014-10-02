/*
 * Copyright(c) 2009 by Christian Dietrich <stettberger@dokucode.de.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or later 
 * as published by the Free Software Foundation.
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

#include "config.h"

#include "core/soft_uart.h"

/*
 * ref. Atmel application note AVR305
 */
void 
soft_uart_putchar(uint8_t c)
{
  uint8_t bitcount = 10;
  register uint8_t tmp = 0;

#define BIT_DELAY (((F_CPU)/(DEBUG_BAUDRATE)-9)/3)
#define BIT_NOP (((F_CPU)/(DEBUG_BAUDRATE)-9)%3)
#if BIT_DELAY > 255
#  error "Bitrate is too low for the software uart"
#endif

#define SOFT_UART_PORT(port) _PORT_CHAR(port)

  asm volatile("com %[txbyte]\n\t"
               "sec\n" /* start bit */
               "0:" "\n\t"
                        "brcc 1f\n\t"
                        "cbi %[port], %[pin]\n\t"
                        "rjmp 2f\n"
               "1:" "\n\t"
                        "sbi %[port], %[pin]\n\t"
                        "nop\n"
               "2:" "\n\t"
                        "ldi %[tmp], %[delay]\n"
               "3: dec %[tmp]\n\t"
                        "brne 3b\n\t"
#if BIT_NOP > 0
                         "nop\n\t"
#endif 
#if BIT_NOP > 1
                         "nop\n\t"
#endif 

                        "lsr %[txbyte]\n\t"
                        "dec %[bitcnt]\n\t"
                        "brne 0b\n\t"
               : /* No output */ 
               : [txbyte] "a" (c), [bitcnt] "a" (bitcount), [tmp] "a" (tmp),
                 [port] "I" (_SFR_IO_ADDR(SOFT_UART_PORT(SOFT_UART_TX_PORT))), 
                 [pin] "I" (SOFT_UART_TX_PIN), [delay] "M" (BIT_DELAY)
               );
}

