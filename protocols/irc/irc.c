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

#ifdef IRC_GREET_SUPPORT
static const char PROGMEM irc_joinmsg_str[] =
    " JOIN :#" CONF_IRC_CHANNEL;

static const char PROGMEM irc_send_greet[] =
    CONF_IRC_GREET_MSG;
#endif	/* IRC_GREET_SUPPORT */

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


#ifdef ECMD_IRC_SUPPORT
static void
irc_handle_ecmd (void)
{
    int16_t len = ecmd_parse_command(STATE->inbuf, STATE->outbuf,
				     ECMD_OUTPUTBUF_LENGTH - 1);

    if ((STATE->reparse = len <= -10))
	len = -len - 10;

    if (len < 0)
	strcpy_P (STATE->outbuf, PSTR ("parse error"));
    else
	STATE->outbuf[len] = 0;

    return;
}
#endif	/* ECMD_IRC_SUPPORT */


static void
irc_handle_message (char *message)
{
    IRCDEBUG ("handle_message: %s\n", message);

#ifdef ECMD_IRC_SUPPORT
    if (*message == '!') {
	strncpy (STATE->inbuf, message + 1, ECMD_INPUTBUF_LENGTH -1);
	STATE->inbuf[ECMD_INPUTBUF_LENGTH - 1] = 0;

	irc_handle_ecmd ();
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

#ifdef IRC_GREET_SUPPORT
	/* :stesie!n=stesie@sudkessel.zerties.org JOIN :#ethersex */
	if (((char *)uip_appdata)[0] == ':'
	    && strstr_P (uip_appdata, irc_joinmsg_str)) {
	    IRCDEBUG ("found join");
	    char *nick = uip_appdata + 1;
	    char *endptr = strchr (nick, '!');

	    if (endptr) {
		*endptr = 0;
		IRCDEBUG ("greeting %s", nick);
		if (strcmp_P (nick, PSTR (CONF_IRC_NICKNAME)))
		    snprintf_P (STATE->outbuf, ECMD_OUTPUTBUF_LENGTH,
				irc_send_greet, nick);
	    }
	}
#endif	/* IRC_GREET_SUPPORT */

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
#ifdef ECMD_IRC_SUPPORT
	STATE->reparse = 0;
#endif
	*STATE->outbuf = 0;
    }

    if (STATE->stage == IRC_SEND_JOIN
	&& uip_acked ())
	STATE->stage ++;

    else if (STATE->stage == IRC_CONNECTED
	     && uip_acked ()) {
	*STATE->outbuf = 0;

#ifdef ECMD_IRC_SUPPORT
	if (STATE->reparse)
	    irc_handle_ecmd ();
#endif
    }

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
