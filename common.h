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

#ifndef _COMMON_H
#define _COMMON_H

#include "shell.h"
#include "sntp_state.h"

#define NULL ((void *)0)

#define HI8(x)  ((uint8_t)((x) >> 8))
#define LO8(x)  ((uint8_t)(x))

#define HTONL(x) ((uint32_t)(((x) & 0xFF000000) >> 24) \
                | (uint32_t)(((x) & 0x00FF0000) >> 8) \
                | (uint32_t)(((x) & 0x0000FF00) << 8) \
                | (uint32_t)(((x) & 0x000000FF) << 24))

#define NTOHL(x) HTONL(x)


/* uip appstate */
typedef union uip_tcp_connection_state {
    struct shell_connection_state_t shell;
} uip_tcp_appstate_t;

typedef union uip_udp_connection_state {
    struct sntp_connection_state_t sntp;
} uip_udp_appstate_t;

#include "uip.h"

#endif
