/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 *     Idea and implementation for char startup mode by
 *     Scott Torborg - storborg@mit.edu - August 2006
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

#ifndef _FOODLOADER_CONFIG_H
#define _FOODLOADER_CONFIG_H

/* only do avr specific configuration when compiling for avr architecture */
#ifdef __AVR__

    #include <avr/version.h>

    /* check for avr-libc version */
    #if __AVR_LIBC_VERSION__ < 10402UL
    #error "newer libc version (>= 1.4.2) needed!"
    #endif

    /* check if cpu speed is defined */
    #ifndef F_CPU
    #error "please define F_CPU!"
    #endif

    /* check if the bootloader start address has been given */
    #if !(defined(BOOT_SECTION_START))
    #error "please define BOOT_SECTION_START as the byte address of bootloader section"
    #endif

    /* cpu specific configuration registers */
    #if defined(__AVR_ATmega8__)
    /* {{{ */
    #define _ATMEGA8

    #define _TIMSK_TIMER1 TIMSK
    #define _UDRIE_UART0 UDRIE
    #define _TXEN_UART0 TXEN
    #define _RXEN_UART0 RXEN
    #define _RXCIE_UART0 RXCIE
    #define _UBRRH_UART0 UBRRH
    #define _UBRRL_UART0 UBRRL
    #define _UCSRA_UART0 UCSRA
    #define _UCSRB_UART0 UCSRB
    #define _UCSRC_UART0 UCSRC
    #define _UCSZ0_UART0 UCSZ0
    #define _UCSZ1_UART0 UCSZ1
    #define _SIG_UART_RECV_UART0 SIG_UART_RECV
    #define _SIG_UART_DATA_UART0 SIG_UART_DATA
    #define _UDR_UART0 UDR
    #define _UDRE_UART0 UDRE
    #define _RXC_UART0 RXC
    #define _IVREG GICR
    #define _TIFR_TIMER1 TIFR

    #define MCUSR MCUCSR

    /* see datasheet! */
    #define _SIG_BYTE_1 0x1e
    #define _SIG_BYTE_2 0x93
    #define _SIG_BYTE_3 0x07

    /* see avrdude configuration */
    #define _AVR910_DEVCODE 0x76

    /* }}} */
    #elif defined(__AVR_ATmega88__)
    /* {{{ */
    #define _ATMEGA88

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
    #define _IVREG MCUCR
    #define _TIFR_TIMER1 TIFR1

    /* see datasheet! */
    #define _SIG_BYTE_1 0x1e
    #define _SIG_BYTE_2 0x93
    #define _SIG_BYTE_3 0x0a

    /* see avrdude configuration */
    #define _AVR910_DEVCODE 0x33

    /* }}} */
    #elif defined(__AVR_ATmega168__)
    /* {{{ */
    #define _ATMEGA168

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
    #define _IVREG MCUCR
    #define _TIFR_TIMER1 TIFR1

    /* see datasheet! */
    #define _SIG_BYTE_1 0x1e
    #define _SIG_BYTE_2 0x94
    #define _SIG_BYTE_3 0x06

    /* see avrdude configuration */
    #define _AVR910_DEVCODE 0x35

    /* }}} */
    #elif defined(__AVR_ATmega32__)
    /* {{{ */
    #define _ATMEGA32

    #define _TIMSK_TIMER1 TIMSK1
    #define _UDRIE_UART0 UDRIE0
    #define _TXEN_UART0 TXEN
    #define _RXEN_UART0 RXEN
    #define _RXCIE_UART0 RXCIE0
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
    #define _TIFR_TIMER1 TIFR

    /* see datasheet! */
    #define _SIG_BYTE_1 0x1e
    #define _SIG_BYTE_2 0x95
    #define _SIG_BYTE_3 0x02

    /* see avrdude configuration */
    #define _AVR910_DEVCODE 0x72

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
    #define _IVREG MCUCR
    #define _TIFR_TIMER1 TIFR1

    /* see datasheet! */
    #define _SIG_BYTE_1 0x1e
    #define _SIG_BYTE_2 0x96
    #define _SIG_BYTE_3 0x09

    /* see avrdude configuration */
    #define _AVR910_DEVCODE 0x02

    /* }}} */
    #else
    #error "this cpu is not supported yet!"
    #endif

#endif /* __AVR__ */

/* debug defines */
#ifndef DEBUG
#define DEBUG 0
#endif

/* do not send boot message by default */
#ifndef SEND_BOOT_MESSAGE
#define SEND_BOOT_MESSAGE 0
#endif

/* bootloader activation methods */

/* 1) activation via jumper */
//#define BOOTLOADER_JUMPER

/* jumper configuration */
#ifndef BOOTLOADER_DDR
#define BOOTLOADER_DDR DDRC
#endif

#ifndef BOOTLOADER_PORT
#define BOOTLOADER_PORT PORTC
#endif

#ifndef BOOTLOADER_PIN
#define BOOTLOADER_PIN PINC
#endif

#ifndef BOOTLOADER_PINNUM
#define BOOTLOADER_PINNUM PINC0
#endif

#define BOOTLOADER_MASK _BV(BOOTLOADER_PINNUM)

/* 2) activation via char */
//#define BOOTLOADER_CHAR

#ifndef BOOTLOADER_ENTRY_CHAR
#define BOOTLOADER_ENTRY_CHAR 'p'
#endif

#ifndef BOOTLOADER_SUCCESS_CHAR
#define BOOTLOADER_SUCCESS_CHAR 'S'
#endif

/* 3) activation via empty reset vector */
//#define BOOTLOADER_RESET_VECTOR


/* uart configuration */
#define UART_BAUDRATE 115200

/* watchdog configuration */
#define HONOR_WATCHDOG_RESET

/* buffer load configuration */
#define BLOCKSIZE SPM_PAGESIZE

/* by default, ignore the exit-bootloader command */
#define EXIT_BOOTLOADER 0

/* use 8 or 16 bit counter, according to the page size of the target device */
#if SPM_PAGESIZE < 256
#   define BUF_T uint8_t
#else
#   define BUF_T uint16_t
#endif

/* version information */
#define VERSION_BYTE_1 '0'
#define VERSION_BYTE_2 '2'


/* user signal configuration */
#define SIGNAL_ENC28J60
#ifdef SIGNAL_ENC28J60

    /* spi defines */
#   ifndef SPI_DDR
#   define SPI_DDR DDRB
#   endif

#   ifndef SPI_PORT
#   define SPI_PORT PORTB
#   endif

#   ifndef SPI_MOSI
#   define SPI_MOSI PB5
#   endif

#   ifndef SPI_MISO
#   define SPI_MISO PB6
#   endif

#   ifndef SPI_SCK
#   define SPI_SCK PB7
#   endif

    /* enc28j60 */
#   ifndef SPI_CS_ENC28J60
#   define SPI_CS_ENC28J60 PB4
#   endif

#   define ENC28J60_CMD_RCR   0x00 /* read control register */
#   define ENC28J60_CMD_WCR   0x40 /* write control register */
#   define ENC28J60_CMD_BFS   0x80 /* address, data */
#   define ENC28J60_CMD_BFC   0xA0 /* address, data */
#   define ENC28J60_CMD_RESET 0xff /* reset enc28j60 */

#   define ENC28J60_REG_ESTAT 0x1d /* status register */
#   define     ENC28J60_REG_ESTAT_CLKRDY   0 /* ready bit in estat */
#   define ENC28J60_REG_ECON1 0x1F
#   define     ENC28J60_ECON1_BSEL0   0
#   define     ENC28J60_ECON1_BSEL1   1
#   define ENC28J60_REG_MIREGADR  (0x14|0x40|0x80)
#   define ENC28J60_REG_MIWRL     (0x16|0x40|0x80)
#   define ENC28J60_REG_MIWRH     (0x17|0x40|0x80)
#   define ENC28J60_REG_MISTAT    (0x0A|0x60|0x80)
#   define     ENC28J60_BUSY   0
#   define ENC28J60_PHY_PHLCON  0x14
#   define     ENC28J60_STRCH   1
#   define     ENC28J60_LFRQ0   2
#   define     ENC28J60_LFRQ1   3
#   define     ENC28J60_LBCFG0  4
#   define     ENC28J60_LBCFG1  5
#   define     ENC28J60_LBCFG2  6
#   define     ENC28J60_LBCFG3  7
#   define     ENC28J60_LACFG0  8
#   define     ENC28J60_LACFG1  9
#   define     ENC28J60_LACFG2  10
#   define     ENC28J60_LACFG3  11

/* masks */
#   define REGISTER_ADDRESS_MASK   0x1F
#   define REGISTER_BANK_MASK      0x60
#   define BANK_MASK               0x03

#endif

/* common macros */
#define HI8(x)  ((uint8_t)((x) >> 8))
#define LO8(x)  ((uint8_t)(x))

#endif /* _FOODLOADER_CONFIG_H */
