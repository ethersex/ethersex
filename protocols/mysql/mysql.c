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

#include "config.h"
#include "protocols/uip/uip.h"
#include "mysql.h"


#define STATE (&uip_conn->appstate.mysql)

static uip_conn_t *mysql_conn;

struct mysql_login_request_t {
    uint8_t packet_len[3];
    uint8_t packet_id;

    uint8_t capabilities[4];
    uint8_t max_packet_size[4];
    uint8_t charset;
};

struct mysql_request_packet_t {
    uint8_t packet_len[3];
    uint8_t packet_id;

    uint8_t command_id;
    char arg[];
};

extern void sha1 (char *dest, char *message, uint32_t len);

static void
mysql_password_hash (char *dest)
{
    char password[sizeof (CONF_MYSQL_PASSWORD)];
    strcpy_P (password, PSTR (CONF_MYSQL_PASSWORD));

    char stage1[20];
    sha1 (stage1, password, (sizeof (CONF_MYSQL_PASSWORD) - 1) * 8);

    char stage2[40];
    memmove (stage2, STATE->u.seed, 20);
    sha1 (stage2 + 20, stage1, 20 * 8);

    sha1 (dest, stage2, 40 * 8);
    for (uint8_t i = 0; i < 20; i ++)
	dest[i] ^= stage1[i];
}


static void
mysql_send_data (uint8_t send_state)
{
    MYDEBUG ("send_data: %d\n", send_state);

    switch (send_state) {
	struct mysql_login_request_t *lr;
	struct mysql_request_packet_t *rp;

    case MYSQL_SEND_LOGIN:
	/* Clear packet until username field. */
	memset (uip_sappdata, 0, 36);

	lr = uip_sappdata;
	lr->packet_id = STATE->packetid;

	/* Long password support (0x01), connect with database set (0x08)*/
	lr->capabilities[0] = 0x09;
	/* We speek 4.1 protocol (0x02), we are an interactive client (not
	   really, but otherwise our times out after 10sec) (0x04),
	   we do 4.1 authentication (0x80) */
	lr->capabilities[1] = 0x86;
	/* Max. packet size = 256 byte */
	lr->max_packet_size[0] = 0xFF;
	lr->charset = 8;	/* Latin1 charset. */

	char *ptr = uip_sappdata + 36;
	strcpy_P (ptr, PSTR (CONF_MYSQL_USERNAME));
	ptr += sizeof (CONF_MYSQL_USERNAME);

	*(ptr ++) = 20;
	mysql_password_hash (ptr);
	ptr += 20;

	strcpy_P (ptr, PSTR (CONF_MYSQL_SCHEMA));
	ptr += sizeof (CONF_MYSQL_SCHEMA);

	lr->packet_len[0] = (ptr - (char*)uip_sappdata) - 4;
	uip_send (uip_sappdata, ptr - (char*) uip_sappdata);
	break;

    case MYSQL_CONNECTED:
	if (!*STATE->u.stmtbuf) {
	    MYDEBUG ("no queries pending.\n");
	    break;
	}

	rp = uip_sappdata;
	rp->packet_len[0] = 1 + strlen (STATE->u.stmtbuf);
	rp->packet_len[1] = 0;
	rp->packet_len[2] = 0;
	rp->packet_id = STATE->packetid = 0;
	rp->command_id = 3;	/* Query */
	strcpy (rp->arg, STATE->u.stmtbuf);

	uip_send (uip_sappdata, 4 + rp->packet_len[0]);
	break;

    default:
	MYDEBUG ("eeek, what?\n");
	uip_abort ();
	break;
    }

    STATE->sent = send_state;
    STATE->packetid ++;
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

    case MYSQL_SEND_LOGIN:
	if (((unsigned char *) uip_appdata)[4] == 0xFE) {
	    MYDEBUG ("server requires multi stage auth (unsupported).\n");
	    return 1;
	}

    case MYSQL_CONNECTED:
	if (((unsigned char *) uip_appdata)[4] == 0xFF) {
	    MYDEBUG ("request failed.\n");
	    return 1;
	}

	/* Clear statement buffer ... */
	*STATE->u.stmtbuf = 0;
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

    if (uip_rexmit()) {
	STATE->packetid --;
	mysql_send_data (STATE->sent);
    }
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
    if (!mysql_conn) {
	MYDEBUG ("no mysql_conn available.\n");
	return 1;
    }

    if (mysql_conn->appstate.mysql.stage < MYSQL_CONNECTED) {
	MYDEBUG ("mysql_conn not in connected state.\n");
	return 1;
    }

    if (*mysql_conn->appstate.mysql.u.stmtbuf) {
	MYDEBUG ("mysql_conn statement buffer busy.\n");
	return 1;
    }

    if (strlen (message) >= MYSQL_STMTBUF_LEN) {
	MYDEBUG ("query too long.\n");
	return 1;
    }

    strcpy(mysql_conn->appstate.mysql.u.stmtbuf, message);
    MYDEBUG ("successfully queued query.\n");
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
    set_CONF_MYSQL_IP(&ip);
    mysql_conn = uip_connect(&ip, HTONS(3306), mysql_main);

    if (! mysql_conn) {
	MYDEBUG ("no uip_conn available.\n");
	return;
    }
}

/*
  -- Ethersex META --
  header(protocols/mysql/mysql.h)
  net_init(mysql_init)
  timer(500, mysql_periodic())

  state_header(protocols/mysql/mysql_state.h)
  state_tcp(struct mysql_connection_state_t mysql)
*/
