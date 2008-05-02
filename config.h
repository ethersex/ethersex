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


/* hd44780 support */
// #define HD44780_SUPPORT

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
// #define FS20_SUPPORT
#define FS20_SUPPORT_SEND
#define FS20_SUPPORT_RECEIVE
#define FS20_SUPPORT_RECEIVE_WS300



#define ECMD_SUPPORT
// #define UECMD_SUPPORT
#define ECMD_PARSER_SUPPORT
// #define ECMD_SENDER_SUPPORT
// #define WATCHCAT_SUPPORT
#define PORTIO_SUPPORT
// #define PORTIO_SIMPLE_SUPPORT
// #define NAMED_PIN_SUPPORT
// #define BOOTP_SUPPORT
// #define BOOTP_TO_EEPROM_SUPPORT
// #define BOOTLOADER_SUPPORT
#define IPV6_SUPPORT
// #define IPV6_STATIC_SUPPORT
// #define BROADCAST_SUPPORT
// #define ONEWIRE_SUPPORT
#define TCP_SUPPORT
#define ICMP_SUPPORT
#define UDP_SUPPORT
// #define DNS_SUPPORT
// #define RC5_SUPPORT
// #define RFM12_SUPPORT
// #define RFM12_RAW_SUPPORT
// #define RFM12_BEACON_SUPPORT
// #define RFM12_CLOUD_SUPPORT
// #define DYNDNS_SUPPORT
// #define SYSLOG_SUPPORT
// #define I2C_SUPPORT
// #define I2C_SLAVE_SUPPORT
// #define CLOCK_SUPPORT
// #define CLOCK_CRYSTAL_SUPPORT
// #define DCF77_SUPPORT
// #define NTP_SUPPORT
// #define NTPD_SUPPORT
// #define CRON_SUPPORT
// #define MDNS_SD_SUPPORT
#define ENC28J60_SUPPORT
// #define STELLA_SUPPORT
// #define TEENSY_SUPPORT
// #define HC595_SUPPORT
// #define HC165_SUPPORT
// #define UDP_ECHO_NET_SUPPORT
// #define ADC_SUPPORT
// #define PS2_SUPPORT
// #define RFM12_LINKBEAT_NET_SUPPORT
// #define ZBUS_LINKBEAT_NET_SUPPORT
// #define UDP_DNS_MCAST_SUPPORT

// #define USART_SUPPORT
// #define ZBUS_SUPPORT
// #define ZBUS_RAW_SUPPORT
// #define YPORT_SUPPORT
// #define MODBUS_SUPPORT

/* crypto stuff */
// #define CRYPTO_SUPPORT
// #define CAST5_SUPPORT
// #define SKIPJACK_SUPPORT
// #define MD5_SUPPORT

/* bootloader config */
#define CONF_BOOTLOAD_DELAY 250           /* five seconds */

// #define TFTP_SUPPORT
// #define TFTPOMATIC_SUPPORT
#define CONF_TFTP_IP uip_ip6addr(ip,0x2001,0x4b88,0x10e4,0x0,0x21a,0x92ff,0xfe32,0x53e3)
#define CONF_TFTP_IMAGE "testsex.bin"
#define CONF_TFTP_KEY "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"

#define CONF_ETHERRAPE_MAC "\xAC\xDE\x48\xFD\x0F\xD0"
#define CONF_ETHERRAPE_IP uip_ip6addr(ip,0x2001,0x6f8,0x1209,0x23,0x0,0x0,0xfe9b,0xee52)
#define CONF_ETHERRAPE_IP6_PREFIX_LEN 64
#define CONF_ETHERRAPE_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_ETHERRAPE_IP4_GATEWAY uip_ipaddr(ip,0,0,0,0)

// #define OPENVPN_SUPPORT
#define CONF_OPENVPN_IP4 uip_ipaddr(ip,10,1,0,5)
#define CONF_OPENVPN_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_OPENVPN_IP4_GATEWAY uip_ipaddr(ip,0,0,0,0)
#define CONF_OPENVPN_KEY "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define CONF_OPENVPN_HMAC_KEY "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

#define CONF_RFM12_IP uip_ip6addr(ip,0x2001,0x6f8,0x1209,0x23,0xaede,0x48ff,0xfe0b,0xee52)
#define CONF_RFM12_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_RFM12_IP6_PREFIX_LEN 64
#define CONF_RFM12_KEY "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"
#define CONF_RFM12_BEACON_ID 23
#define CONF_RfM12_CLOUD_DNS "rfmnet.dyn.metafnord.de"

#define CONF_I2C_SLAVE_ADDR 0x23

#define CONF_ZBUS_IP uip_ip6addr(ip,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x23)
#define CONF_ZBUS_IP4_NETMASK uip_ipaddr(ip,255,255,255,0)
#define CONF_ZBUS_IP6_PREFIX_LEN 64
#define CONF_ZBUS_KEY "\x23\x23\x42\x42\x55\x55\x23\x23\x42\x42"

#define CONF_DNS_SERVER uip_ip6addr(ip,0x2001,0x6f8,0x1209,0x0,0x0,0x0,0x0,0x2)
#define CONF_SYSLOG_SERVER uip_ipaddr(ip,10,0,0,1)

#define CONF_DYNDNS_USERNAME "jochen"
#define CONF_DYNDNS_PASSWORD "bier42"
#define CONF_DYNDNS_HOSTNAME "ethsex1"

#define CONF_HOSTNAME "ethersex"

#endif /* _CONFIG_H */
