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

#ifndef _SHELL_H
#define _SHELL_H

#include <stdint.h>

//#include "sntp.h"

enum shell_state_t {
    SHELL_IDLE = 0,
    SHELL_WELCOME,
    SHELL_STATUS,
    SHELL_UPTIME,
    SHELL_HELP,
    SHELL_EXIT,
    SHELL_SENSORS,
};

typedef struct shell_connection_state_t {
    enum shell_state_t state;
} uip_tcp_appstate_t;

typedef union udp_connection_state {
    // struct sntp_connection_state_t sntp;
} uip_udp_appstate_t;


#define UIP_APPCALL shell_main
#define UIP_UDP_APPCALL shell_handle_udp

void shell_init(void);
void shell_main(void);
void shell_send_response(void);
void shell_handle_udp(void);

#endif
