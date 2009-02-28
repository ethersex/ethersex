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
#include "mysql.h"


#define STATE (&uip_conn->appstate.mysql)

static uip_conn_t *mysql_conn;


static void
mysql_send_data (uint8_t send_state)
{
    MYDEBUG ("send_data: %d\n", send_state);

    switch (send_state) {

    default:
	MYDEBUG ("eeek, what?\n");
	uip_abort ();
	break;
    }

    STATE->sent = send_state;
}


static uint8_t
mysql_parse (void)
{
    MYDEBUG ("mysql_parse stage=%d\n", STATE->stage);

    if (((unsigned char *) uip_appdata)[2] != 0) {
	MYDEBUG ("incoming packet far too large!\n");
	return 1;		/* Closes connection. */
    }

    uint16_t packet_len = ((unsigned char *) uip_appdata)[0]
	+ (((unsigned char *) uip_appdata)[1] << 8);
    uint8_t packet_id = ((unsigned char *) uip_appdata)[3];

    MYDEBUG ("... packet's len=%d, id=%d.\n", packet_len, packet_id);

    if (packet_id != STATE->packetid) {
	MYDEBUG ("packet is out of line, expected %d.\n", STATE->packetid);
	return 1;
    }

    unsigned char *ptr = uip_appdata + 4;

    switch (STATE->stage) {
    case MYSQL_WAIT_GREETING:
	/* Parse/expect a MySQL Server Greeting packet. */
	if (*ptr != MYSQL_SERVER_PROTO_VERSION) {
	    MYDEBUG ("expected server protocol version %d, got %d\n",
		     MYSQL_SERVER_PROTO_VERSION, *ptr);
	    return 1;
	}

	ptr ++;			/* now at `server version' */
	ptr += strlen ((char*) ptr);
	ptr ++;			/* now at `thread id' */
	ptr += 4;		/* now at `scramble buf' */

	memcpy (STATE->u.seed, ptr, 8);
	ptr += 8;

	if (*ptr != 0) {
	    MYDEBUG ("first part of seed no zero-terminated.\n");
	    return 1;
	}

	ptr += 19;		/* now at 2nd scramble buf */
	memcpy (STATE->u.seed + 8, ptr, 12);
	ptr += 12;

	if (*ptr != 0) {
	    MYDEBUG ("second part of seed no zero-terminated.\n");
	    return 1;
	}

	MYDEBUG ("found valid server greeting!\n");
	break;

    default:
	MYDEBUG ("eeek, no comprendo!\n");
	return 1;
    }

    /* Jippie, let's enter next stage if we haven't reached connected. */
    if (STATE->stage != MYSQL_CONNECTED)
	STATE->stage ++;

    STATE->packetid ++;
    return 0;
}



static void
mysql_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	MYDEBUG ("connection aborted\n");
	mysql_conn = NULL;
    }

    if (uip_closed()) {
	MYDEBUG ("connection closed\n");
	mysql_conn = NULL;
    }

    if (uip_connected()) {
	MYDEBUG ("new connection\n");
	STATE->stage = MYSQL_WAIT_GREETING;
	STATE->sent = MYSQL_WAIT_GREETING;
	STATE->packetid = 0;
    }

    if (uip_newdata() && uip_len) {
#ifdef DEBUG_MYSQL
	MYDEBUG ("received data: %s\n", uip_appdata);
	for (uint16_t i = 0; i < uip_len; i ++)
	    debug_putchar (((unsigned char *) uip_appdata)[i]);
	debug_putchar (10);
#endif

	if (mysql_parse ()) {
	    uip_close ();		/* Parse error */
	    return;
	}
    }

    if (uip_rexmit())
	mysql_send_data (STATE->sent);

    else if ((STATE->stage > STATE->sent || STATE->stage == MYSQL_CONNECTED)
	     && (uip_newdata()
		 || uip_acked()
		 || uip_connected()))
	mysql_send_data (STATE->stage);
    else if (STATE->stage == MYSQL_CONNECTED
	     && uip_poll() && *STATE->u.stmtbuf)
	mysql_send_data(STATE->stage);

}

uint8_t
mysql_send_message(char *message)
{
    if (!mysql_conn) return 1;
    if (mysql_conn->appstate.mysql.stage < MYSQL_CONNECTED) return 1;
    if (*mysql_conn->appstate.mysql.u.stmtbuf) return 1;
    if (strlen (message) >= MYSQL_STMTBUF_LEN) return 1;

    strcpy(mysql_conn->appstate.mysql.u.stmtbuf, message);
    return 0;
}


void
mysql_periodic(void)
{
    if (!mysql_conn)
	mysql_init();
}


void
mysql_init(void)
{
    MYDEBUG ("initializing mysql client\n");

    uip_ipaddr_t ip;
    CONF_MYSQL_IP;
    mysql_conn = uip_connect(&ip, HTONS(3306), mysql_main);

    if (! mysql_conn) {
	MYDEBUG ("no uip_conn available.\n");
	return;
    }
}
