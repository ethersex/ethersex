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
#if defined(BOOTLOADER_SUPPORT) || !defined(ENC28J60_SUPPORT)
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

#ifdef ENC28J60_SUPPORT
#define __LLH_LEN  14
#else /* RFM12_SUPPORT */
#define __LLH_LEN  0
#endif

#ifdef OPENVPN_SUPPORT
#  define UIP_MULTI_STACK        1

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
#    define OPENVPN_LLH_LEN (__LLH_LEN + 40 + 8)
#  else
#    define OPENVPN_LLH_LEN (__LLH_LEN + 20 + 8)
#  endif

#  ifdef MD5_SUPPORT
#    define OPENVPN_HMAC_LLH_LEN   16
#  else
#    define OPENVPN_HMAC_LLH_LEN   0
#  endif

#  ifdef CAST5_SUPPORT
#    define OPENVPN_CRYPT_LLH_LEN  16 /* 8 bytes IV + 8 bytes packet id */
#  else
#    define OPENVPN_CRYPT_LLH_LEN  0
#  endif

#  ifdef OPENVPN_OUTER
#  else
#    define OPENVPN_INNER
#    define UIP_CONF_LLH_LEN  (OPENVPN_LLH_LEN + OPENVPN_CRYPT_LLH_LEN \
			       + OPENVPN_HMAC_LLH_LEN)
#  endif

#else /* !OPENVPN_SUPPORT */
#  define UIP_MULTI_STACK        0
#  define UIP_CONF_LLH_LEN  __LLH_LEN
#endif


/**
 * Some forward declarations
 *
 */

struct __uip_conn;
typedef struct __uip_conn uip_conn_t;
struct __uip_udp_conn;
typedef struct __uip_udp_conn uip_udp_conn_t;

/**
 * Repressentation of an IP address.
 *
 */
typedef u16_t uip_ip4addr_t[2];
typedef u16_t uip_ip6addr_t[8];
#if UIP_CONF_IPV6
typedef uip_ip6addr_t uip_ipaddr_t;
#else /* UIP_CONF_IPV6 */
typedef uip_ip4addr_t uip_ipaddr_t;
#endif /* UIP_CONF_IPV6 */

enum {
  STACK_MAIN,
#ifdef OPENVPN_SUPPORT
  STACK_OPENVPN,
#endif
  
  /* STACK_LEN must be the last! */
  STACK_LEN
};



#if UIP_MULTI_STACK
#  include "uip_multi.h"

#else
#  ifdef STACK_NAME
#    undef STACK_NAME
#    define STACK_NAME(a) uip_ ## a /* keep common function names, since no
	  			       multi-stack support.*/
#  endif

#  define uip_stack_get_active()   (0)
#  define uip_stack_set_active(i)  (0)

#endif

#endif /* __UIP_CONF_H__ */
