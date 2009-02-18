/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../uip/uip.h"
#include "jabber.h"
#include "../ecmd_parser/ecmd.h"

static void
jabber_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	JABDEBUG ("connection aborted\n");
    }

    if (uip_closed()) {
	JABDEBUG ("connection closed\n");
    }

    if (uip_connected()) {
	JABDEBUG ("new connection\n");
    }

    if (uip_newdata()) {
	JABDEBUG ("received data\n");
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected()) {

	JABDEBUG ("send data\n");
    }
}


void
jabber_init(void)
{
    JABDEBUG ("initializing jabber client\n");

    uip_ipaddr_t ip;
    CONF_JABBER_IP;
    uip_conn_t *conn = uip_connect(&ip, HTONS(5222), jabber_main);

    if (! conn) {
	JABDEBUG ("no uip_conn available.\n");
	return;
    }
}
