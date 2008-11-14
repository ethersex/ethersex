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

/* network controller hardware bug defines */
#define ENC28J60_REV4_WORKAROUND

/* Include pinning.c as output of m4 scripts in pinning sub-directory.
   Have a look there for MCU specific configuration options. */
#include "pinning.c"

/* ps/2 interrupts */
#define PS2_PCMSK PCMSK0
#define PS2_PCIE PCIE0
#define PS2_INTERRUPT SIG_PIN_CHANGE0

/* global version defines */
#define VERSION_STRING "0.2"

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

/* use watchdog only when not debugging */
#ifndef DEBUG
#   define USE_WATCHDOG
#endif

/* macros */
#ifdef USE_WATCHDOG
#   define wdt_kick() wdt_reset()
#else
#   define wdt_kick()
#endif


/* Map the IP address configuration to use in network.c
   (and that is assigned to the primary stack). */

#ifdef ENC28J60_SUPPORT
#  define CONF_ETHERRAPE_MAC		CONF_ENC_MAC
#  define CONF_ETHERRAPE_IP		CONF_ENC_IP
#  define CONF_ETHERRAPE_IP4_NETMASK	CONF_ENC_IP4_NETMASK

#elif defined (RFM12_SUPPORT)
#  define CONF_ETHERRAPE_IP		CONF_RFM12_IP
#  define CONF_ETHERRAPE_IP4_NETMASK	CONF_RFM12_IP4_NETMASK

#elif defined (ZBUS_SUPPORT)
#  define CONF_ETHERRAPE_IP		CONF_ZBUS_IP
#  define CONF_ETHERRAPE_IP4_NETMASK	CONF_ZBUS_IP4_NETMASK

#elif defined (USB_NET_SUPPORT)
#  define CONF_ETHERRAPE_IP		CONF_USB_NET_IP
#  define CONF_ETHERRAPE_IP4_NETMASK	CONF_USB_NET_IP4_NETMASK
#endif

/* Figure out whether we need access to EEPROM:

   - ECMD without TEENSY (IP address configuration etc.)
   - BOOTP with to-EEPROM-feature */
#if defined(ECMD_PARSER_SUPPORT) && (!defined(TEENSY_SUPPORT))	\
  || defined(BOOTP_SUPPORT) && defined(BOOTP_TO_EEPROM_SUPPORT)
#  define EEPROM_SUPPORT 1
#endif


/* Figure out whether we need CRC_SUPPORT: */
#if defined(EEPROM_SUPPORT)			\
  || defined(ONEWIRE_SUPPORT)			\
  || defined(TFTP_SUPPORT)
#  define CRC_SUPPORT 1
#endif


#endif /* _CONFIG_H */
