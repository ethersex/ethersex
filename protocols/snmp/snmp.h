/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#ifndef _SNMP_H
#define _SNMP_H

#include "config.h"

#define SNMP_VERSION1_VALUE 0

#ifndef SNMP_COMMUNITY_STRING
#define SNMP_COMMUNITY_STRING "public"
#endif

#define SNMP_TYPE_INTEGER     0x02
#define SNMP_TYPE_STRING      0x04
#define SNMP_TYPE_NULL        0x05
#define SNMP_TYPE_OID         0x06
#define SNMP_TYPE_SEQUENCE    0x30
#define SNMP_TYPE_COUNTER     0x41
#define SNMP_TYPE_GAUGE       0x42
#define SNMP_TYPE_TIMETICKS   0x43
#define SNMP_TYPE_ENDOFMIB    0x82
#define SNMP_TYPE_GETREQ      0xa0
#define SNMP_TYPE_GETNEXTREQ  0xa1
#define SNMP_TYPE_GETRESP     0xa2

#define SNMP_ERR_NONE         0x00
#define SNMP_ERR_NO_SUCH_NAME 0x02

/* Buffer overflow protection :
 *     
 * SNMP_MAX_OID_BUFFERSIZE:
 * Maximum stack size allowed to use as OID buffer.
 * 
 * SNMP_MAX_BIND_COUNT:
 * Maximum number of bindings per request.
 * This value is used to prevent an overflow of the
 * output buffer. The intend is that the data of
 * the worst case reaction output * SNMP_MAX_BIND_COUNT
 * fit in the network output buffer
 */
#define SNMP_MAX_OID_BUFFERSIZE 64
#define SNMP_MAX_BIND_COUNT     3

/* OID: 1.3.6.1.4.1. */
#define SNMP_OID_ENTERPRISES "\x2b\x06\x01\x04\x01"

#ifdef SNMP_UCDEXPERIMENTAL_SUPPORT
/* OID: 1.3.6.1.4.1.2021.13.23. */
#define SNMP_OID_ETHERSEX SNMP_OID_ENTERPRISES "\x8f\x65\x0d\x17"
#else
/* OID: 1.3.6.1.4.1.39967 */
#define SNMP_OID_ETHERSEX SNMP_OID_ENTERPRISES "\x82\xb8\x1f"
#endif

struct snmp_varbinding
{
  uint8_t store_len;
  uint8_t len;
  uint8_t *data;
};

typedef uint8_t(*snmp_reaction_callback_t) (uint8_t * ptr,
                                            struct snmp_varbinding * bind,
                                            void *userdata);

typedef uint8_t(*snmp_next_callback_t) (uint8_t * ptr,
                                        struct snmp_varbinding * bind);

struct snmp_reaction
{
  const char *obj_name;
  snmp_reaction_callback_t cb;
  void *userdata;
  snmp_next_callback_t ncb;
};

extern const struct snmp_reaction snmp_reactions[];

#endif /* _SNMP_H */
