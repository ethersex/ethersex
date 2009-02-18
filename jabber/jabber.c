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


static const char PROGMEM jabber_stream_text[] =
    "<?xml version='1.0'?>"
    "<stream:stream version='1.0' "
    "xmlns:stream='http://etherx.jabber.org/streams' "
    "xmlns='jabber:client' to='" CONF_JABBER_HOSTNAME "' "
    "from='" CONF_HOSTNAME "' xml:lang='en' >";


#define JABBER_SEND(str) do {			  \
	memcpy_P (uip_sappdata, str, sizeof (str));     \
	uip_send (uip_sappdata, sizeof (str) - 1);      \
    } while(0)



static void
jabber_send_data (uint8_t send_state)
{
    JABDEBUG ("send_data: %d\n", send_state);

    switch (send_state) {
    case JABBER_OPEN_STREAM:
	JABBER_SEND (jabber_stream_text);
	break;

    default:
	JABDEBUG ("eeek, what?\n");
	uip_abort ();
	break;
    }

    STATE->sent = send_state;
}


static uint8_t
jabber_parse (void)
{
    JABDEBUG ("jabber_parse stage=%d\n", STATE->stage);

    switch (STATE->stage) {
    case JABBER_OPEN_STREAM:
	if (strstr_P (uip_appdata, PSTR ("<stream:stream")) == NULL) {
	    JABDEBUG ("<stream:stream not found in reply.  stop.");
	    return 1;
	}
	break;

    default:
	JABDEBUG ("eeek, no comprendo!\n");
	return 1;
    }

    /* Jippie, let's enter next stage if we haven't reached connected. */
    if (STATE->stage != JABBER_CONNECTED)
	STATE->stage ++;
    return 0;
}



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
	STATE->stage = JABBER_OPEN_STREAM;
	STATE->sent = JABBER_INIT;
    }

    if (uip_newdata() && uip_len) {
	/* Zero-terminate */
	((char *) uip_appdata)[uip_len] = 0;
	JABDEBUG ("received data: %s\n", uip_appdata);

	if (jabber_parse ()) {
	    uip_close ();		/* Parse error */
	    return;
	}

    }

    if (uip_rexmit())
	jabber_send_data (STATE->sent);

    else if (STATE->stage > STATE->sent
	     && (uip_newdata()
		 || uip_acked()
		 || uip_connected()))
	jabber_send_data (STATE->stage);

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
