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

#ifndef _SYSLOG_H
#define _SYSLOG_H

#include <avr/pgmspace.h>

#include "config.h"
#include "syslog_state.h"
#include "uip/uip.h"

#define SYSLOG_UDP_PORT 514

#define SYSLOG_STATE_MESSAGE 0
#define SYSLOG_STATE_SENSOR_MESSAGE 1

/* global variables */
extern uip_ipaddr_t syslog_server;

void syslog_message(PGM_P str);
#define syslog_message_P(x) syslog_message(PSTR(x))
void syslog_sensor(uint8_t num, uint8_t state);
void syslog_handle_conn(void);

#endif
