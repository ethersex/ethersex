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

/* use watchdog only when not debugging */
#ifndef DEBUG
#   define USE_WATCHDOG
#endif

/* network controller hardware bug defines */
#define ENC28J60_REV4_WORKAROUND

/* controller type */
#define HD44780_ORIGINAL /* original hd44780 */
// #define HD44780_KS0067B  /* compatibility mode for ks0067b */

/* readback support, check busy flag instead of just waiting for a timeout */
// #define HD44780_READBACK

/* if defined, use portc instead of port a */
// #define HD44780_USE_PORTC


/* Include pinning.c as output of m4 scripts in pinning sub-directory.
   Have a look there for MCU specific configuration options. */
#include "pinning.c"


/* rfm12 module interrupt line */
#ifndef RFM12_INT_PIN 
#define RFM12_INT_PIN INT0
#endif

#ifndef RFM12_INT_SIGNAL
#define RFM12_INT_SIGNAL SIG_INTERRUPT0
#endif

/* ps/2 interrupts */
#define PS2_PCMSK PCMSK0
#define PS2_PCIE PCIE0
#define PS2_INTERRUPT SIG_PIN_CHANGE0

/* Comment this out to get an us layout */
#define PS2_GERMAN_LAYOUT

/* Number of the hc 595 registers */
#define HC595_REGISTERS 5

#define HC165_INVERSE_OUTPUT 1
/* Number of the hc165 registers */
#define HC165_REGISTERS 1


/* global version defines */
#define VERSION_STRING "0.2"

/* uart defines */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

/* configure duplex mode */
#define NET_FULL_DUPLEX 0

/* configure main callback function for uip */
#define UIP_APPCALL network_handle_tcp
#define UIP_UDP_APPCALL network_handle_udp

/* onewire support */
#define ONEWIRE_PARASITE

/* rc5 support */
#define RC5_QUEUE_LENGTH 10

/* fs20 support */
#define FS20_SUPPORT_SEND
#define FS20_SUPPORT_RECEIVE
#define FS20_SUPPORT_RECEIVE_WS300

/* bootloader config */
#define CONF_BOOTLOAD_DELAY 250           /* five seconds */

#include "autoconf.h"

#endif /* _CONFIG_H */
