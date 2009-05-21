/*
 * Copyright (c) 2009 by Bernd Stellwag <burned@zerties.org>
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
#include "protocols/ecmd/parser.h"
#include "irc.h"

#define STATE (&uip_conn->appstate.irc)

static uip_conn_t *irc_conn;


static const char PROGMEM irc_send_usernick[] =
    "USER " CONF_IRC_USERNAME " 42 * :" CONF_IRC_REALNAME "\n"
    "NICK " CONF_IRC_NICKNAME "\n";

static const char PROGMEM irc_send_altnick[] =
    "NICK " CONF_IRC_NICKNAME "_\n";

static const char PROGMEM irc_send_join[] =
    "JOIN #" CONF_IRC_CHANNEL "\n";

static const char PROGMEM irc_privmsg_str[] =
    "PRIVMSG #" CONF_IRC_CHANNEL " :";


#define IRC_SEND(str) do {				\
	memcpy_P (uip_sappdata, str, sizeof (str));     \
	uip_send (uip_sappdata, sizeof (str) - 1);      \
    } while(0)


static void
irc_send_data (uint8_t send_state)
{
    IRCDEBUG ("send_data: %d\n", send_state);

    switch (send_state) {
    case IRC_SEND_USERNICK:
	IRC_SEND (irc_send_usernick);
	break;

    case IRC_SEND_ALTNICK:
	IRC_SEND (irc_send_altnick);
	break;

    case IRC_SEND_JOIN:
	IRC_SEND (irc_send_join);
	break;

    case IRC_CONNECTED:
	if (*STATE->outbuf) {
	    uip_slen = sprintf_P (uip_sappdata, PSTR(
				      "PRIVMSG #" CONF_IRC_CHANNEL " :%s\n"),
				  STATE->outbuf);
	}
	else {
	    IRCDEBUG ("successfully joined, nothing to do.");
	}
	break;
    }

    STATE->sent = send_state;
}


static void
irc_handle_message (char *message)
{
    IRCDEBUG ("handle_message: %s\n", message);

#ifdef ECMD_IRC_SUPPORT
    if (*message == '!') {
	message ++;		/* Skip exclamation mark. */

	int16_t len = ecmd_parse_command(message, STATE->outbuf,
					 ECMD_OUTPUTBUF_LENGTH - 1);

	if (len <= -10) {
	    IRCDEBUG ("irc_ecmd doesn't support multiple reply lines (yet)\n");
	    len = -len - 10;
	}

	if (len < 0)
	    strcpy_P (STATE->outbuf, PSTR ("parse error"));
	else
	    STATE->outbuf[len] = 0;

	return;
    }
#endif
}


static uint8_t
irc_parse (void)
{
    char *message;
    IRCDEBUG ("ircparse stage=%d\n", STATE->stage);

    switch (STATE->stage) {
    case IRC_SEND_USERNICK:
	if (strstr_P (uip_appdata, PSTR (" 433 "))) {
	    IRCDEBUG ("nickname already in use, try alternative one.");
	    STATE->stage = IRC_SEND_ALTNICK;
	    return 0;
	}

	if (strstr_P (uip_appdata, PSTR (" 001 "))) {
	    IRCDEBUG ("remote host accepted connection.");
	    STATE->stage = IRC_SEND_JOIN;
	    return 0;
	}

	if (strstr_P (uip_appdata, PSTR ("NOTICE AUTH"))) {
	    IRCDEBUG ("ignoring auth fluff ...");
	    return 0;
	}
	break;

    case IRC_SEND_ALTNICK:
	if (strstr_P (uip_appdata, PSTR (" 443 "))) {
	    IRCDEBUG ("nickname already also in use, stop.");
	    return 1;
	}

	if (strstr_P (uip_appdata, PSTR (" 001 "))) {
	    IRCDEBUG ("remote host accepted connection.");
	    STATE->stage = IRC_SEND_JOIN;
	    return 0;
	}
	break;

    case IRC_SEND_JOIN:
    case IRC_CONNECTED:
	message = strstr_P (uip_appdata, irc_privmsg_str);
	if (message) {
	    message += sizeof (irc_privmsg_str) - 1;
	    irc_handle_message (message);
	}
	return 0;
    }

    /* Reset connection */
    IRCDEBUG ("unknown server reply, stop.");
    return 1;
}

static void
irc_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	IRCDEBUG ("connection aborted\n");
	irc_conn = NULL;
    }

    if (uip_closed()) {
	IRCDEBUG ("connection closed\n");
	irc_conn = NULL;
    }

    if (uip_connected()) {
	IRCDEBUG ("new connection\n");
	STATE->stage = IRC_SEND_USERNICK;
	STATE->sent = IRC_SEND_INIT;
	*STATE->outbuf = 0;
    }

    if (STATE->stage == IRC_SEND_JOIN
	&& uip_acked ())
	STATE->stage ++;

    else if (STATE->stage == IRC_CONNECTED
	     && uip_acked ())
	*STATE->outbuf = 0;

    else if (uip_newdata() && uip_len) {
	((char *) uip_appdata)[uip_len] = 0;
	IRCDEBUG ("received data: %s\n", uip_appdata);

	if (irc_parse ()) {
	    uip_close ();	/* Parse error */
	    return;
	}
    }

    if (uip_rexmit ())
	irc_send_data (STATE->sent);

    else if ((STATE->stage > STATE->sent || STATE->stage == IRC_CONNECTED)
	     && (uip_newdata()
		 || uip_acked()
		 || uip_connected()))
	irc_send_data (STATE->stage);

    else if (STATE->stage == IRC_CONNECTED && uip_poll() && *STATE->outbuf)
	irc_send_data (STATE->stage);
}


void
irc_periodic(void)
{
  if (! irc_conn)
      irc_init();
}


void
irc_init(void)
{
    IRCDEBUG ("initializing irc client\n");

    uip_ipaddr_t ip;
    set_CONF_IRC_IP(&ip);
    irc_conn = uip_connect(&ip, HTONS(CONF_IRC_PORT), irc_main);

    if (! irc_conn) {
	IRCDEBUG ("no uip_conn available.\n");
	return;
    }
}

/*
  -- Ethersex META --
  header(protocols/irc/irc.h)
  net_init(irc_init)
  timer(500, irc_periodic())
*/
