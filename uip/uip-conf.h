/* uip configuration */

#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <inttypes.h>
#include "../config.h"

/**
 * 8 bit datatype
 *
 * This typedef defines the 8-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint8_t u8_t;

/**
 * 16 bit datatype
 *
 * This typedef defines the 16-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint16_t u16_t;

/**
 * Statistics datatype
 *
 * This typedef defines the dataype used for keeping statistics in
 * uIP.
 *
 * \hideinitializer
 */
typedef unsigned short uip_stats_t;

/**
 * Maximum number of TCP connections.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_CONNECTIONS 3

/**
 * Maximum number of listening TCP ports.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_LISTENPORTS 10

/**
 * uIP buffer size.
 *
 * \hideinitializer
 */
#define UIP_CONF_BUFFER_SIZE     NET_MAX_FRAME_LENGTH

/**
 * CPU byte order.
 *
 * \hideinitializer
 */
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

/**
 * Logging on or off
 *
 * \hideinitializer
 */
#ifdef DEBUG
#   define UIP_CONF_LOGGING         1
#else
#   define UIP_CONF_LOGGING         0
#endif

/** TCP support on or off */
#ifdef TCP_SUPPORT
#   define UIP_CONF_TCP             1
#else
#   define UIP_CONF_TCP             0
#endif

/** UDP support on or off */
#ifdef UDP_SUPPORT
#   define UIP_CONF_UDP             1
#else
#   define UIP_CONF_UDP             0
#endif

#define UIP_CONF_UDP_CONNS            5

/**
 * UDP checksums on or off
 *
 * \hideinitializer
 */
#if defined(IPV6_SUPPORT) || !defined(BOOTLOADER_SUPPORT)
#define UIP_CONF_UDP_CHECKSUMS   1
#else
#define UIP_CONF_UDP_CHECKSUMS   0
#endif

/**
 * uIP statistics on or off
 *
 * \hideinitializer
 */
#ifdef BOOTLOADER_SUPPORT
#define UIP_CONF_STATISTICS      0
#else
#define UIP_CONF_STATISTICS      1
#endif

#ifdef IPV6_SUPPORT
#  define UIP_CONF_IPV6          1
#else
#  define UIP_CONF_IPV6          0
#endif

#ifdef BROADCAST_SUPPORT
#  define UIP_CONF_BROADCAST     1
#else
#  define UIP_CONF_BROADCAST     0
#endif

#define UIP_ARCH_ADD32           0
#define UIP_ARCH_CHKSUM          0


#ifdef OPENVPN_SUPPORT
     /* The header of the link layer (of the inner stack) consists of:
      *
      *                                       IPv4          IPv6
      *
      *     actual link layer (ethernet)        14            14
      *     IP header of OpenVPN stack          20            40
      *     UDP header of OpenVPN stack          8             8
      *   ----------------------------------------------------------
      *     total                               42            62
      */
#  if UIP_CONF_IPV6
#    define OPENVPN_LLH_LEN 62
#  else
#    define OPENVPN_LLH_LEN 42
#  endif

#  ifdef CAST5_SUPPORT
#    define OPENVPN_CRYPT_LLH_LEN  16 /* 8 bytes IV + 8 bytes packet id */
#  else
#    define OPENVPN_CRYPT_LLH_LEN  0
#  endif

#  ifdef OPENVPN_OUTER
#  else
#    define OPENVPN_INNER
#    define UIP_CONF_LLH_LEN  (OPENVPN_LLH_LEN + OPENVPN_CRYPT_LLH_LEN)
#  endif
#endif


#endif /* __UIP_CONF_H__ */
