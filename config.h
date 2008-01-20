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
#error "newer libc version (>= 1.4.4) needed!"
#endif

/* check if cpu speed is defined */
#ifndef F_CPU
#error "please define F_CPU!"
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
#elif defined(__AVR_ATmega8__)
/* {{{ */
#define _ATMEGA8

#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _TIFR_TIMER1 TIFR
#define _EIMSK GICR

/* on ATmega8 we connect rfm12 directly to SPI. */
#define SPI_CS_RFM12_DDR DDRB
#define SPI_CS_RFM12_PORT PORTB
#define SPI_CS_RFM12 PB2

/* SPI-pinout differs from atmega32/644 one. */
#define SPI_MOSI PB3
#define SPI_MISO PB4
#define SPI_SCK PB5


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
#define _EIMSK EIMSK

#define _TCCR2_PRESCALE TCCR2B
#define _OUTPUT_COMPARE_IE2 OCIE2B
#define _OUTPUT_COMPARE_REG2 OCR2B
#define _SIG_OUTPUT_COMPARE2 SIG_OUTPUT_COMPARE2B
#define _TIMSK_TIMER2 TIMSK2

/* workaround for avr-libc devs not being able to decide how these registers
 * should be named... */
#ifdef SPCR0
    #define _SPCR0 SPCR0
#else
    #define _SPCR0 SPCR
#endif

#ifdef SPE0
    #define _SPE0 SPE0
#else
    #define _SPE0 SPE
#endif

#ifdef MSTR0
    #define _MSTR0 MSTR0
#else
    #define _MSTR0 MSTR
#endif

#ifdef SPSR0
    #define _SPSR0 SPSR0
#else
    #define _SPSR0 SPSR
#endif

#ifdef SPIF0
    #define _SPIF0 SPIF0
#else
    #define _SPIF0 SPIF
#endif

#ifdef SPDR0
    #define _SPDR0 SPDR0
#else
    #define _SPDR0 SPDR
#endif

#ifdef SPI2X0
    #define _SPI2X0 SPI2X0
#else
    #define _SPI2X0 SPI2X
#endif

#define _TIFR_TIMER1 TIFR1

/* }}} */
#else
#error "this cpu isn't supported yet!"
#endif

/* use watchdog only when not debugging */
#ifndef DEBUG
#   define USE_WATCHDOG
#endif

/* network controller hardware bug defines */
#define ENC28J60_REV4_WORKAROUND

/* bootloader */
#undef BOOTLOADER_SECTION
#define BOOTLOADER_SECTION 0xe000 /* atmega644 with 4096 words bootloader */

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
#ifndef SPI_CS_NET
#define SPI_CS_NET PB4
#endif

/* port the dataflash CS is attached to */
#ifndef SPI_CS_DF
#define SPI_CS_DF PB1
#endif

/* port the rfm12 module CS is attached to */
#ifndef SPI_CS_RFM12_DDR
#define SPI_CS_RFM12_DDR DDRC
#endif

#ifndef SPI_CS_RFM12_PORT
#define SPI_CS_RFM12_PORT PORTC
#endif

#ifndef SPI_CS_RFM12
#define SPI_CS_RFM12 PC3
#endif

/* rfm12 module interrupt line */
#ifndef RFM12_INT_PIN 
#define RFM12_INT_PIN INT0
#endif

#ifndef RFM12_INT_SIGNAL
#define RFM12_INT_SIGNAL SIG_INTERRUPT0
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
#define VERSION_STRING "0.2"

/* uart defines */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

/* configure duplex mode */
#define NET_FULL_DUPLEX 0

/* configure global data buffer */
#ifdef _ATMEGA8
  /* there isn't that much RAM on ATmega8, reduce uip_buf size. */
#  define NET_MAX_FRAME_LENGTH 192
#else
#  define NET_MAX_FRAME_LENGTH 640
#endif

/* configure main callback function for uip */
#define UIP_APPCALL network_handle_tcp
#define UIP_UDP_APPCALL network_handle_udp

/* onewire support */
#define ONEWIRE_PINNUM PD6
#define ONEWIRE_PIN PIND
#define ONEWIRE_DDR DDRD
#define ONEWIRE_PORT PORTD
#define ONEWIRE_PARASITE

/* rc5 support */
#define RC5_SEND_PINNUM PD4
#define RC5_SEND_PORT PORTD
#define RC5_SEND_DDR DDRD
#define RC5_QUEUE_LENGTH 10

/* fs20 support */
// #define FS20_SUPPORT

#define FS20_SUPPORT_SEND
#define FS20_SEND_PINNUM PB2
#define FS20_SEND_DDR DDRB
#define FS20_SEND_PORT PORTB

#define FS20_SUPPORT_RECEIVE
/* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
#define FS20_RECV_PINNUM PB3
#define FS20_RECV_DDR DDRB
#define FS20_RECV_PORT PORTB

#define FS20_SUPPORT_RECEIVE_WS300

/* hd44780 support */

// #define HD44780_SUPPORT

/* controller type */
// #define HD44780_ORIGINAL /* original hd44780 */
#define HD44780_KS0067B  /* compatibility mode for ks0067b */

/* readback support, check busy flag instead of just waiting for a timeout */
#define HD44780_READBACK

/* if defined, use portc instead of port a */
// #define HD44780_USE_PORTC

/* select port for lcd below */
#ifdef HD44780_USE_PORTC
    #define HD44780_CTRL_PORT C
    #define HD44780_DATA_PORT C
    #define HD44780_RS PC0
    #define HD44780_RW PC1
    #define HD44780_EN PC2
    #define HD44780_D4 PC3
    #define HD44780_D5 PC4
    #define HD44780_D6 PC5
    #define HD44780_D7 PC6
    #define HD44780_DATA_SHIFT 3
#else
    #define HD44780_CTRL_PORT A
    #define HD44780_DATA_PORT A
    #define HD44780_RS PA0
    #define HD44780_RW PA1
    #define HD44780_EN PA2
    #define HD44780_D4 PA3
    #define HD44780_D5 PA4
    #define HD44780_D6 PA5
    #define HD44780_D7 PA6
    #define HD44780_DATA_SHIFT 3
#endif

#define ECMD_SUPPORT
// #define ECMD_SENDER_SUPPORT
// #define WATCHCAT_SUPPORT
// #define PORTIO_SUPPORT
// #define NAMED_PIN_SUPPORT
// #define TETRIRAPE_SUPPORT
// #define BOOTP_SUPPORT
// #define BOOTP_TO_EEPROM_SUPPORT
// #define BOOTLOADER_SUPPORT
#define IPV6_SUPPORT
// #define BROADCAST_SUPPORT
// #define ONEWIRE_SUPPORT
// #define TCP_SUPPORT
// #define ICMP_SUPPORT
#define UDP_SUPPORT
// #define DNS_SUPPORT
// #define RC5_SUPPORT
#define RFM12_SUPPORT
// #define DYNDNS_SUPPORT
// #define SYSLOG_SUPPORT
// #define I2C_SUPPORT
// #define I2C_SLAVE_SUPPORT
// #define CLOCK_SUPPORT
// #define CLOCK_CRYSTAL_SUPPORT
// #define NTP_SUPPORT
// #define NTPD_SUPPORT
// #define ENC28J60_SUPPORT
// #define ZBUS_SUPPORT
#define SENSOR_RFM12_SUPPORT
// #define STELLA_SUPPORT
#define TEENSY_SUPPORT
// #define UDP_ECHO_NET_SUPPORT
// #define RFM12_LINKBEAT_NET_SUPPORT

/* crypto stuff */
#define CRYPTO_SUPPORT
// #define CAST5_SUPPORT
#define SKIPJACK_SUPPORT
// #define MD5_SUPPORT

/* bootloader config */
#define CONF_BOOTLOAD_DELAY 250           /* five seconds */

// #define TFTP_SUPPORT
// #define TFTPOMATIC_SUPPORT
#define CONF_TFTP_IP uip_ip6addr(ip,0x2001,0x4b88,0x10e4,0x0,0x21a,0x92ff,0xfe32,0x53e3)
#define CONF_TFTP_IMAGE "testsex.bin"
#define CONF_TFTP_KEY "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"

#define CONF_ETHERRAPE_MAC "\xAC\xDE\x48\x0B\xEE\x52"
#define CONF_ETHERRAPE_IP uip_ip6addr(ip,0x2001,0x6f8,0x1209,0x23,0x0,0x0,0xfe2b,0xee52)
#define CONF_ETHERRAPE_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_ETHERRAPE_IP4_GATEWAY uip_ipaddr(ip,0,0,0,0)

// #define OPENVPN_SUPPORT
#define CONF_OPENVPN_IP4 uip_ipaddr(ip,10,1,0,5)
#define CONF_OPENVPN_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_OPENVPN_IP4_GATEWAY uip_ipaddr(ip,0,0,0,0)
#define CONF_OPENVPN_KEY "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define CONF_OPENVPN_HMAC_KEY "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

#define CONF_RFM12_IP uip_ip6addr(ip,0x2001,0x6f8,0x1209,0x23,0xaede,0x48ff,0xfe1b,0xee52)
#define CONF_RFM12_KEY "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"

#define CONF_DNS_SERVER uip_ipaddr(ip,10,0,0,1)
#define CONF_SYSLOG_SERVER uip_ipaddr(ip,10,0,0,1)

#define CONF_DYNDNS_USERNAME "jochen"
#define CONF_DYNDNS_PASSWORD "bier42"
#define CONF_DYNDNS_HOSTNAME "ethsex1.dyn.metafnord.de"

#endif /* _CONFIG_H */
