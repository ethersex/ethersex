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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <avr/version.h>

/* check for avr-libc version */
#if __AVR_LIBC_VERSION__ < 10404UL
#error newer libc version (>= 1.4.4) needed!
#endif

/* check if cpu speed is defined */
#ifndef F_CPU
#error please define F_CPU!
#endif

/* cpu specific configuration registers */
#if defined(__AVR_ATmega32__)
/* {{{ */
#define _ATMEGA32

#define _TIMSK_TIMER1 TIMSK1
#define _UDRIE_UART0 UDRIE
#define _TXEN_UART0 TXEN
#define _RXEN_UART0 RXEN
#define _RXCIE_UART0 RXCIE
#define _UBRRH_UART0 UBRRH
#define _UBRRL_UART0 UBRRL
#define _UCSRA_UART0 UCSRA
#define _UCSRB_UART0 UCSRB
#define _UCSRC_UART0 UCSRC
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#define _SIG_UART_RECV_UART0 SIG_USART_RECV
#define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _UDR_UART0 UDR
#define _UDRE_UART0 UDRE
#define _RXC_UART0 RXC
#define _IVREG MCUCR
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MCUSR MCUCSR
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _TIFR_TIMER1 TIFR

/* }}} */
#elif defined(__AVR_ATmega644__)
/* {{{ */
#define _ATMEGA644

#define _TIMSK_TIMER1 TIMSK1
#define _UDRIE_UART0 UDRIE0
#define _TXEN_UART0 TXEN0
#define _RXEN_UART0 RXEN0
#define _RXCIE_UART0 RXCIE0
#define _UBRRH_UART0 UBRR0H
#define _UBRRL_UART0 UBRR0L
#define _UCSRA_UART0 UCSR0A
#define _UCSRB_UART0 UCSR0B
#define _UCSRC_UART0 UCSR0C
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#define _SIG_UART_RECV_UART0 SIG_USART_RECV
#define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _UDR_UART0 UDR0
#define _UDRE_UART0 UDRE0
#define _RXC_UART0 RXC0
#define _TXC_UART0 TXC0
#define _IVREG MCUCR
#define _SPCR0 SPCR0
#define _SPE0 SPE0
#define _MSTR0 MSTR0
#define _SPSR0 SPSR0
#define _SPIF0 SPIF0
#define _SPDR0 SPDR0
#define _SPI2X0 SPI2X0
#define _TIFR_TIMER1 TIFR1

/* }}} */
#else
#error this cpu isn't supported yet!
#endif

/* use watchdog only when not debugging */
#ifndef DEBUG
#   define USE_WATCHDOG
#endif

/* network controller hardware bug defines */
#define ENC28J60_REV4_WORKAROUND

/* bootloader */
#if defined(_ATMEGA32)
#   define BOOTLOADER_SECTION 0x7c00 /* atmega32 with 512 words bootloader */
#elif defined(_ATMEGA644)
#   define BOOTLOADER_SECTION 0xf800 /* atmega644 with 1024 words bootloader */
#else
#   warning bootloader entry point unknown...
#endif

/* spi defines */
#ifndef SPI_DDR
#define SPI_DDR DDRB
#endif

#ifndef SPI_PORT
#define SPI_PORT PORTB
#endif

#ifndef SPI_MOSI
#define SPI_MOSI PB5
#endif

#ifndef SPI_MISO
#define SPI_MISO PB6
#endif

#ifndef SPI_SCK
#define SPI_SCK PB7
#endif

/* port the enc28j60 is attached to
 * ATTENTION: EITHER USE SS OR MAKE SURE, SS IS PULLED HIGH OR AN OUTPUT! */
#ifndef SPI_CS
#define SPI_CS PB4
#endif

/* enc28j60 int line */
#ifndef INT_PIN_NAME
#define INT_PIN_NAME PB3
#endif

#ifndef INT_PORT
#define INT_PORT PORTB
#endif

#ifndef INT_PIN
#define INT_PIN PINB
#endif

#ifndef INT_DDR
#define INT_DDR DDRB
#endif

/* enc28j60 wol line */
#ifndef WOL_PIN_NAME
#define WOL_PIN_NAME PB2
#endif

#ifndef WOL_PORT
#define WOL_PORT PORTB
#endif

#ifndef WOL_PIN
#define WOL_PIN PINB
#endif

#ifndef WOL_DDR
#define WOL_DDR DDRB
#endif

/* global version defines */
#define VERSION_WORD 0x0002
#define VERSION_STRING "0.2"

/* uart defines */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

/* configure duplex mode */
#define FULL_DUPLEX 0

/* configure global data buffer */
#define MAX_FRAME_LENGTH 640

/* support for 74HC165 */
//#define USE_74HC165

/* onewire support */
#define ONEWIRE_SUPPORT
#define ONEWIRE_PINNUM PD6
#define ONEWIRE_PIN PIND
#define ONEWIRE_DDR DDRD
#define ONEWIRE_PORT PORTD

#endif /* _CONFIG_H */
