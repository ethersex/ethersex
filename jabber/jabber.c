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

#include "known_buddies.c"


static const char PROGMEM jabber_stream_text[] =
    "<?xml version='1.0'?>"
    "<stream:stream xmlns:stream='http://etherx.jabber.org/streams' "
    "xmlns='jabber:client' to='" CONF_JABBER_HOSTNAME "' "
    "from='" CONF_HOSTNAME "' xml:lang='en' >";

static const char PROGMEM jabber_get_auth_text[] =
    "<iq id='ga' type='get'><query xmlns='jabber:iq:auth'>"
    "<username>" CONF_JABBER_USERNAME "</username></query></iq>";

static const char PROGMEM jabber_set_auth_text[] =
    "<iq id='sa' type='set'><query xmlns='jabber:iq:auth'>"
    "<resource>" CONF_JABBER_RESOURCE "</resource>"
    "<username>" CONF_JABBER_USERNAME "</username>"
    "<password>" CONF_JABBER_PASSWORD "</password></query></iq>";

static const char PROGMEM jabber_set_presence_text[] =
    /* Set the presence */
    "<presence><priority>1</priority></presence>"
    ;

static const char PROGMEM jabber_startup_text[] =
    /* This message must NOT be longer than STATE->outbuf,
       be careful ;) */
    "Your Ethersex '"CONF_HOSTNAME"' is now UP :)";

#define JABBER_SEND(str) do {			  \
	memcpy_P (uip_sappdata, str, sizeof (str));     \
	uip_send (uip_sappdata, sizeof (str) - 1);      \
    } while(0)

#define STATE (&uip_conn->appstate.jabber)


static uip_conn_t *jabber_conn;


#ifdef ECMD_JABBER_SUPPORT
static void
jabber_parse_ecmd (const char *from, char *message)
{
    if (strlen (from) > TARGET_BUDDY_MAXLEN) {
	JABDEBUG ("parse_ecmd: from addr too long!\n");
	return;
    }

    JABDEBUG("message from: %s\n", from);
    uint8_t i = 0;
    uint8_t auth = 1;
    while (1) {
      char *known_buddy_ptr = (char *)pgm_read_word(&jabber_known_buddies[i]);
      if (known_buddy_ptr == NULL)
        break;
      auth = 0;
      if (strcmp_P(from, known_buddy_ptr) == 0) {
        auth = 1;
        break;
      }
      i++;
    }

    JABDEBUG("ecmd: authentificated %s: %d\n", from, auth);
    strcpy (STATE->target, from);

    if (!auth) { /* authentification failed */
      strcpy_P(STATE->outbuf, PSTR("permission denied"));
    } else {
      int16_t len = ecmd_parse_command(message, STATE->outbuf,
                                       ECMD_OUTPUTBUF_LENGTH - 1);
      if (len <= -10) {
          JABDEBUG ("jabber_ecmd doesn't support multiple reply lines (yet)\n");
          len = -len - 10;
      }

      if (len < 0)
          strcpy_P (STATE->outbuf, PSTR ("parse error"));
      else
          STATE->outbuf[len] = 0;
    }
}
#endif	/* ECMD_JABBER_SUPPORT */


static void
jabber_send_data (uint8_t send_state)
{
    JABDEBUG ("send_data: %d\n", send_state);

    switch (send_state) {
    case JABBER_OPEN_STREAM:
	JABBER_SEND (jabber_stream_text);
	break;

    case JABBER_GET_AUTH:
	JABBER_SEND (jabber_get_auth_text);
	break;

    case JABBER_SET_AUTH:
	JABBER_SEND (jabber_set_auth_text);
	break;

    case JABBER_SET_PRESENCE:
	JABBER_SEND (jabber_set_presence_text);
	break;

    case JABBER_CONNECTED:
	if (*STATE->outbuf) {
	    uip_slen = sprintf_P (uip_sappdata, PSTR(
				      "<message to='%s' type='chat'>"
				      "<body>%s</body></message>"),
		       STATE->target, STATE->outbuf);
        } else {
	    JABDEBUG ("idle, don't know what to send right now ...\n");
        }
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

    case JABBER_GET_AUTH:
	if (strstr_P (uip_appdata, PSTR ("<password/>")) == NULL) {
	    JABDEBUG ("<password/> not found in reply.  stop.");
	    return 1;
	}
	break;

    case JABBER_SET_AUTH:
	if (strstr_P (uip_appdata, PSTR ("result")) == NULL) {
	    JABDEBUG ("authentication failed.  stop.");
	    return 1;
	}

	JABDEBUG ("jippie, we successfully authenticated to the server!\n");
	break;

    case JABBER_SET_PRESENCE:
    case JABBER_CONNECTED:
#ifdef ECMD_JABBER_SUPPORT
	if (strncmp_P (uip_appdata, PSTR ("<mess"), 5) == 0) {
	    const char *from = strstr_P (uip_appdata, PSTR ("from="));
	    char *body = strstr_P (uip_appdata, PSTR ("<body>"));

	    if (!from || !body) {
		JABDEBUG ("received invalid message.\n");
		break;		/* Ignore, not really fatal. */
	    }

	    from += 6;		/* skip from=' */
	    body += 6;		/* skip body tag. */

	    char *ptr = strstr_P (uip_appdata, PSTR ("</bod"));
	    if (! ptr) {
		JABDEBUG ("received incomplete message, buffer overrun?\n");
		break;
	    }
	    *ptr = 0;		/* terminate body text. */

	    ptr = strchr (from, '/');
            if (!ptr) ptr = strchr(from, '\'');
            if (!ptr) ptr = strchr(from, '\"');
	    if (! ptr) {
		JABDEBUG ("end of from addr not found!\n");
		break;
	    }
	    *ptr = 0;		/* chop off resource name */

	    jabber_parse_ecmd (from, body);
	} else
#endif	/* ECMD_JABBER_SUPPORT */

	JABDEBUG ("got something, but no idea how to parse it :(\n");
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
        jabber_conn = NULL;
    }

    if (uip_closed()) {
	JABDEBUG ("connection closed\n");
        jabber_conn = NULL;
    }

    if (uip_connected()) {
	JABDEBUG ("new connection\n");
	STATE->stage = JABBER_OPEN_STREAM;
	STATE->sent = JABBER_INIT;

	strcpy_P (STATE->target, PSTR(CONF_JABBER_BUDDY));
	strcpy_P (STATE->outbuf, jabber_startup_text);
    }

    if (uip_acked() && STATE->stage == JABBER_CONNECTED)
	*STATE->outbuf = 0;

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

    else if ((STATE->stage > STATE->sent || STATE->stage == JABBER_CONNECTED)
	     && (uip_newdata()
		 || uip_acked()
		 || uip_connected()))
	jabber_send_data (STATE->stage);
    else if (STATE->stage == JABBER_CONNECTED && uip_poll() && *STATE->outbuf)
        jabber_send_data(STATE->stage);

}

uint8_t
jabber_send_message(char *message)
{
  if (!jabber_conn) return 0;
  if (*jabber_conn->appstate.jabber.outbuf) return 0;

  /* Send message to the default buddy */
  strcpy_P (STATE->target, PSTR(CONF_JABBER_BUDDY));

  memcpy(jabber_conn->appstate.jabber.outbuf, message, 
         sizeof(jabber_conn->appstate.jabber.outbuf));

  jabber_conn->appstate.jabber.outbuf
    [sizeof(jabber_conn->appstate.jabber.outbuf) -1] = 0;
  
  return 1;
}

void 
jabber_periodic(void)
{
  if (!jabber_conn) {
    jabber_init();
  }
}


void
jabber_init(void)
{
    JABDEBUG ("initializing jabber client\n");

    uip_ipaddr_t ip;
    CONF_JABBER_IP;
    jabber_conn = uip_connect(&ip, HTONS(5222), jabber_main);

    if (! jabber_conn) {
	JABDEBUG ("no uip_conn available.\n");
	return;
    }
}
