/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _CONFIG_H
#define _CONFIG_H

/* define these menu choices before including autoconf.h */
#define BMP085_OSS_0 0
#define BMP085_OSS_1 1
#define BMP085_OSS_2 2
#define BMP085_OSS_3 3

#include "autoconf.h"

/* network controller hardware bug defines */
#define ENC28J60_REV4_WORKAROUND
#define ENC28J60_REV5_WORKAROUND
#define ENC28J60_REV6_WORKAROUND  /* rev is 6; but microchip calls it B7 */

/* global version defines */
#define USE_BUILDDATE_VERSION 1
#define USE_GIT_VERSION 2
#define USE_RELEASE_VERSION 3

#if VERSION_STRING_CHOICE == USE_BUILDDATE_VERSION
    #define VERSION_STRING __DATE__ " " __TIME__
#else
#if VERSION_STRING_CHOICE == USE_GIT_VERSION
    #define VERSION_STRING VERSION_GIT
#else
#if VERSION_STRING_CHOICE == USE_RELEASE_VERSION
    #define xstr_(s) str_(s)
    #define str_(s) #s
    #define VERSION_MAJOR 0
    #define VERSION_MINOR 2
    #define VERSION_STRING xstr_(VERSION_MAJOR)"."xstr_(VERSION_MINOR)" ("VERSION_GIT")"
#else
    #warning No Version choosen
#endif
#endif
#endif

/* configure duplex mode */
#define NET_FULL_DUPLEX 0

/* configure main callback function for uip */
#define UIP_APPCALL if (uip_conn->callback != NULL) uip_conn->callback
#define UIP_UDP_APPCALL if (uip_udp_conn->callback) uip_udp_conn->callback

/* onewire support */
#define ONEWIRE_PARASITE

/* rc5 support */
#define RC5_QUEUE_LENGTH 10

/* bootloader config */
#define CONF_BOOTLOAD_DELAY 250           /* five seconds */

/* rfm12 config */
#define RFM12_FREQ_433920	433920
#define RFM12_FREQ_868300	868300
#define RFM12_FREQ_869775	869775
#define RFM12_FREQ_869800	869800
#define RFM12_FREQ_869825	869825
#define RFM12_FREQ_869850	869850
#define RFM12_FREQ_869875	869875
#define RFM12_FREQ_869900	869900
#define RFM12_FREQ_869925	869925

#define ARCH_AVR	1
#define ARCH_HOST	2

#if ARCH == ARCH_HOST
#include "core/host/host.h"
#endif

/* Include pinning.c as output of m4 scripts in pinning sub-directory.
   Have a look there for MCU specific configuration options. */
#include "pinning.c"

/* use watchdog only when not debugging */
#ifndef DEBUG
#   define USE_WATCHDOG
#endif

/* macros */
#ifdef USE_WATCHDOG
#   include <avr/wdt.h>
#   define wdt_kick() wdt_reset()
#else
#   define wdt_kick()
#endif


/* Map the IP address configuration to use in network.c
   (and that is assigned to the primary stack). */

#ifdef ENC28J60_SUPPORT
#  define set_CONF_ETHERSEX_MAC(ip)		set_CONF_ENC_MAC(ip)
#  define set_CONF_ETHERSEX_IP(ip)		set_CONF_ENC_IP(ip)
#  define set_CONF_ETHERSEX_IP4_NETMASK(ip)	set_CONF_ENC_IP4_NETMASK(ip)
#  define CONF_ETHERSEX_MAC		CONF_ENC_MAC

#elif defined (TAP_SUPPORT)
#  define set_CONF_ETHERSEX_MAC(ip)		set_CONF_TAP_MAC(ip)
#  define set_CONF_ETHERSEX_IP(ip)		set_CONF_TAP_IP(ip)
#  define set_CONF_ETHERSEX_IP4_NETMASK(ip)	set_CONF_TAP_IP4_NETMASK(ip)
#  define CONF_ETHERSEX_MAC			CONF_TAP_MAC

#elif defined (RFM12_IP_SUPPORT)
#  define set_CONF_ETHERSEX_IP(ip)		set_CONF_RFM12_IP(ip)
#  define set_CONF_ETHERSEX_IP4_NETMASK(ip)	set_CONF_RFM12_IP4_NETMASK(ip)

#elif defined (ZBUS_SUPPORT)
#  define set_CONF_ETHERSEX_IP(ip)		set_CONF_ZBUS_IP(ip)
#  define set_CONF_ETHERSEX_IP4_NETMASK(ip)	set_CONF_ZBUS_IP4_NETMASK(ip)

#elif defined (USB_NET_SUPPORT)
#  define set_CONF_ETHERSEX_IP(ip)		set_CONF_USB_NET_IP(ip)
#  define set_CONF_ETHERSEX_IP4_NETMASK(ip)	set_CONF_USB_NET_IP4_NETMASK(ip)
#endif

/* ADC Reference Flags */
#define ADC_AREF	0
#define ADC_AVCC	0x40
#define ADC_1_1	    0x80
#define ADC_2_56    0xC0

/* Figure out whether we need access to EEPROM:

   - ECMD without TEENSY (IP address configuration etc.)
   - BOOTP with to-EEPROM-feature
   - STELLA with eeprom load/write support */
#if (defined(ECMD_PARSER_SUPPORT) && (!defined(TEENSY_SUPPORT)))  \
  || (defined(BOOTP_SUPPORT) && defined(BOOTP_TO_EEPROM_SUPPORT)) \
  || (defined(STELLA_SUPPORT) && !defined(TEENSY_SUPPORT))
#  define EEPROM_SUPPORT 1
#endif


/* Figure out whether we need CRC_SUPPORT: */
#if defined(EEPROM_SUPPORT)			\
  || defined(ONEWIRE_SUPPORT)			\
  || defined(VFS_INLINE_SUPPORT)		\
  || defined(TFTP_SUPPORT)
#  define CRC_SUPPORT 1
#endif

#if defined(VFS_SUPPORT) && defined(VFS_INLINE_SUPPORT)	  \
  && !defined(VFS_SD_SUPPORT) && !defined(VFS_DF_SUPPORT) \
  && !defined(VFS_EEPROM_SUPPORT)			  \
  && !defined(VFS_EEPROM_RAW_SUPPORT)			  \
  && !defined(VFS_DC3840_SUPPORT)
#  define VFS_TEENSY 1
#endif



#endif /* _CONFIG_H */
