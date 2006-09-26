/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include <avr/pgmspace.h>

#include "config.h"
#include "uip/uip_arp.h"
#include "uip/uip.h"

/* define uart mode (8N1) */
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
/* {{{ */
/* in atmega8 and atmega32, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC (_BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1))

/* }}} */
#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega644__)
/* {{{ */
/* in the new controller generation, this isn't needed any more */
#define UART_UCSRC (_BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0))

/* }}} */
#else
#error this cpu isn't supported by uart.h yet!
#endif

#if (UART_BAUDRATE == 115200 && F_CPU == 16000000)
#   define UART_UBRR 8 /* 16mhz, 115200 */
#elif (UART_BAUDRATE == 115200 && F_CPU == 20000000)
#   define UART_UBRR 10 /* 20mhz, 115200 */
#else
#   define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)
#endif

/* prototypes */

/* init uart with given baudrate and mode, enable rx and tx */
void uart_init(void);

/* output one byte, do busy waiting while transfer is in progress */
void uart_putc(uint8_t data);

/* output data from flash while byte is no-null */
void uart_putf(void *buffer);

/** output a lower bytes nibble in hex */
void uart_puthexnibble(uint8_t data);

/** output a byte in hex */
void uart_puthexbyte(uint8_t data);

/** output a byte in decimal */
void uart_putdecbyte(uint8_t data);

/** output eol (\r\n) */
void uart_eol(void);

/** output a formatted mac address */
void uart_puts_mac(struct uip_eth_addr *mac);

/** output a formatted ip address */
void uart_puts_ip(uip_ipaddr_t *ip);

/* macros, for direct string output from flash */
#define uart_puts_P(s) uart_putf(PSTR(s))

#endif
