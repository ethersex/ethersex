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

#ifndef NETWORK_STATE_H
#define NETWORK_STATE_H

#include "uip/psock.h"
#include "uip/pt.h"
#include "ethcmd.h"
#include "httpd_state.h"
#include "sntp_state.h"
#include "syslog_state.h"
#include "fc_state.h"

/* uip appstate */
typedef union uip_tcp_connection_state {
    struct ethcmd_connection_state_t ethcmd;
    struct httpd_connection_state_t httpd;
} uip_tcp_appstate_t;

/* attention: first byte MUST be transmit_state! */
typedef union uip_udp_connection_state {
    struct sntp_connection_state_t sntp;
    struct syslog_connection_state_t syslog;
    struct fc_connection_state_t fc;
} uip_udp_appstate_t;

#endif
