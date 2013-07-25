/*
 * Copyright (c) 2009 Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2009 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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
#include "core/eeprom.h"
#include "jabber.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "known_buddies.c"


#ifdef JABBER_EEPROM_SUPPORT
static const char PROGMEM jabber_stream_text[] =
  "<?xml version='1.0'?>"
  "<stream:stream xmlns:stream='http://etherx.jabber.org/streams' "
  "xmlns='jabber:client' to='%s' " "from='" CONF_HOSTNAME "' xml:lang='en' >";

static const char PROGMEM jabber_get_auth_text[] =
  "<iq id='ga' type='get'><query xmlns='jabber:iq:auth'>"
  "<username>%s</username></query></iq>";

static const char PROGMEM jabber_set_auth_text[] =
  "<iq id='sa' type='set'><query xmlns='jabber:iq:auth'>"
  "<resource>%s</resource>"
  "<username>%s</username>" "<password>%s</password></query></iq>";

#ifdef JABBER_LAST_SUPPORT
static const char PROGMEM jabber_last_text[] =
  "<iq type='result' id='%s' to='%s' from='%s@%s/%s'>"
  "<query xmlns='jabber:iq:last' seconds='%i'/>" "</iq>";
#endif /* JABBER_LAST_SUPPORT */

#ifdef JABBER_VERSION_SUPPORT
static const char PROGMEM jabber_version_text[] =
  "<iq type='result' id='%s' to='%s' from='%s@%s/%s'>"
  "<query xmlns='jabber:iq:version'>"
  "<name>" CONF_HOSTNAME "</name>"
  "<version>" VERSION_STRING "</version>"
  "<os>" CONF_JABBER_VERSION_OS "</os>" "</query>" "</iq>";
#endif /* JABBER_VERSION_SUPPORT */

char jabber_user[JABBER_VALUESIZE];
char jabber_pass[JABBER_VALUESIZE];
char jabber_resrc[JABBER_VALUESIZE];
char jabber_host[JABBER_VALUESIZE];

#else

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

#ifdef JABBER_LAST_SUPPORT
static const char PROGMEM jabber_last_text[] =
  "<iq type='result' id='%s' to='%s' from='"
  CONF_JABBER_USERNAME "@" CONF_JABBER_HOSTNAME "/" CONF_JABBER_RESOURCE "'>"
  "<query xmlns='jabber:iq:last' seconds='%i'/>" "</iq>";
#endif /* JABBER_LAST_SUPPORT */

#ifdef JABBER_VERSION_SUPPORT
static const char PROGMEM jabber_version_text[] =
  "<iq type='result' id='%s' to='%s' from='"
  CONF_JABBER_USERNAME "@" CONF_JABBER_HOSTNAME "/" CONF_JABBER_RESOURCE "'>"
  "<query xmlns='jabber:iq:version'>"
  "<name>" CONF_HOSTNAME "</name>"
  "<version>" VERSION_STRING "</version>"
  "<os>" CONF_JABBER_VERSION_OS "</os>" "</query>" "</iq>";
#endif /* JABBER_VERSION_SUPPORT */
#endif /* JABBER_EEPROM_SUPPORT */

static const char PROGMEM jabber_set_presence_text[] =
  /* Set the presence */
  "<presence><priority>1</priority></presence>";

static const char PROGMEM jabber_startup_text[] =
  /* This message must NOT be longer than STATE->outbuf,
   * be careful ;) */
  "Your Ethersex '" CONF_HOSTNAME "' is now UP :)";


#define JABBER_SEND_BUFLEN (sizeof(UIP_BUFSIZE)-UIP_IPTCPH_LEN-UIP_LLH_LEN-1)
#define JABBER_SEND(...) {                                           \
    int len;                                                         \
    len = snprintf_P(uip_sappdata, JABBER_SEND_BUFLEN, __VA_ARGS__); \
    JABDEBUG("send:%s\n", (((char *)uip_sappdata)[len] = 0,          \
                            uip_sappdata));                          \
    uip_send(uip_sappdata, len);                                     \
  }

#ifdef JABBER_EEPROM_SUPPORT
#define JABBER_SEND_E(x,...) JABBER_SEND(x,__VA_ARGS__)
#else
#define JABBER_SEND_E(x,...) JABBER_SEND(x)
#endif

#define STATE (&uip_conn->appstate.jabber)

static uip_conn_t *jabber_conn;


#ifdef ECMD_JABBER_SUPPORT
static void
jabber_parse_ecmd(char *message)
{
  int16_t remain = sizeof(STATE->outbuf) - 1;
  int16_t written = 0;

  while (remain > 0)
  {
    int16_t len = ecmd_parse_command(message, STATE->outbuf + written, remain);
    if (is_ECMD_AGAIN(len))
    {
      len = ECMD_AGAIN(len);
      written += len;
      remain -= len;
      if (remain)
      {
        STATE->outbuf[written++] = '\n';
        remain--;
      }
      continue;
    }
    else if (is_ECMD_ERR(len))
    {
      strncpy_P(STATE->outbuf, PSTR("parse error"), sizeof(STATE->outbuf));
      len = 11;
    }
    written = len;
    break;
  }

  STATE->outbuf[written] = 0;
}
#endif /* ECMD_JABBER_SUPPORT */


static void
jabber_send_data(uint8_t send_state, uint8_t action)
{
  JABDEBUG("send_data: %d action: %d\n", send_state, action);

  switch (send_state)
  {
    case JABBER_OPEN_STREAM:
      JABBER_SEND_E(jabber_stream_text, jabber_host);
      break;

    case JABBER_GET_AUTH:
      JABBER_SEND_E(jabber_get_auth_text, jabber_user);
      break;

    case JABBER_SET_AUTH:
      JABBER_SEND_E(jabber_set_auth_text, jabber_resrc, jabber_user,
                    jabber_pass);
      break;

    case JABBER_SET_PRESENCE:
      JABBER_SEND(jabber_set_presence_text);
      break;

    case JABBER_CONNECTED:
      switch (action)
      {
        case JABBER_ACTION_NONE:
          break;

        case JABBER_ACTION_MESSAGE:
          if (*STATE->outbuf)
          {
            uip_slen = snprintf_P(uip_sappdata, JABBER_SEND_BUFLEN,
                                  PSTR("<message to='%s' type='chat'>"
                                       "<body>%s</body></message>"),
                                  STATE->target, STATE->outbuf);
          }
          break;

#ifdef JABBER_VERSION_SUPPORT
        case JABBER_ACTION_VERSION:
          JABBER_SEND(jabber_version_text, STATE->actionid, STATE->target
#ifdef JABBER_EEPROM_SUPPORT
                      , jabber_user, jabber_host, jabber_resrc
#endif
            );
          break;
#endif /* JABBER_VERSION_SUPPORT */

#ifdef JABBER_LAST_SUPPORT
        case JABBER_ACTION_LAST:
        {
          // change iqlasttime if you ever whant dynamic values
          uint16_t iqlasttime = CONF_JABBER_LAST_VALUE;
          JABBER_SEND(jabber_last_text, STATE->actionid, STATE->target,
#ifdef JABBER_EEPROM_SUPPORT
                      jabber_user, jabber_host, jabber_resrc,
#endif
                      iqlasttime);
          break;
        }
#endif /* JABBER_LAST_SUPPORT */

        default:
          JABDEBUG("idle, don't know what to send right now\n");
      }
      break;

    default:
      JABDEBUG("state invalid\n");
      uip_abort();
      break;
  }

  STATE->sent = send_state;
}


/* Copy ID from incoming <iq type='get'> message to our STATE. */
static uint8_t
jabber_extract_id(void)
{
  char *idptr = strstr_P(uip_appdata, PSTR("id='"));
  if (idptr)
  {
    idptr += 4;
    JABDEBUG("id=' found %i\n", idptr);

    char *idendptr = strchr(idptr, '\'');
    if (idendptr)
    {
      uint8_t idlength = idendptr - idptr;

      if (idlength > 15)
        JABDEBUG("id too long: %i\n", idlength);

      else
      {
        JABDEBUG("endquote found %i\n", idendptr);
        memmove(STATE->actionid, idptr, idlength);
        STATE->actionid[idlength] = 0;
        JABDEBUG("given id: %s\n", STATE->actionid);
      }
    }

    return 0;
  }

  return 1;                     /* Failed. */
}

static uint8_t
jabber_extract_from(void)
{
  const char *from = strstr_P(uip_appdata, PSTR("from="));
  if (!from)
    return 1;

  from += 6;                    /* skip from=' */

  const char *resource_end = strchr(from, '/');
  if (!resource_end)
  {
    JABDEBUG("from addr resource not found\n");
    return 1;
  }

  const char *endptr = strchr(from, '\'');
  if (!endptr)
    endptr = strchr(from, '\"');
  if (!endptr)
  {
    JABDEBUG("end of from addr not found\n");
    return 1;
  }

  uint8_t jid_len = resource_end - from;
  uint8_t len = endptr - from;
  if (len + 1 > TARGET_BUDDY_MAXLEN)
  {
    JABDEBUG("extract_from: from addr too long\n");
    return 1;
  }

  uint8_t auth = 1;
  for (uint8_t i = 0;
       i < (sizeof(jabber_known_buddies) / sizeof(jabber_known_buddies[0]));
       ++i)
  {
    char *jidlist_ptr = (char *) pgm_read_word(&jabber_known_buddies[i]);
    auth = strncmp_P(from, jidlist_ptr, jid_len) == 0;
    if (auth == 1)
      break;
  }

  JABDEBUG("authentificated %s: %d\n", from, auth);
  if (!auth)
    return 2;                   /* Permission denied. */

  memmove(STATE->target, from, len);
  STATE->target[len] = 0;

  JABDEBUG("message from: %s\n", STATE->target);
  return 0;                     /* Looks good. */
}

static uint8_t
jabber_parse(void)
{
  JABDEBUG("jabber_parse stage=%d\n", STATE->stage);

  switch (STATE->stage)
  {
    case JABBER_OPEN_STREAM:
      if (strstr_P(uip_appdata, PSTR("<stream:stream")) == NULL)
      {
        JABDEBUG("<stream:stream not found in reply.  stop.");
        return 1;
      }
      break;

    case JABBER_GET_AUTH:
      if (strstr_P(uip_appdata, PSTR("<password/>")) == NULL)
      {
        JABDEBUG("<password/> not found in reply.  stop.");
        return 1;
      }
      break;

    case JABBER_SET_AUTH:
      if (strstr_P(uip_appdata, PSTR("result")) == NULL)
      {
        JABDEBUG("authentication failed.  stop.");
        return 1;
      }

      JABDEBUG("jippie, we successfully authenticated to the server\n");
      break;

    case JABBER_SET_PRESENCE:
    case JABBER_CONNECTED:
#ifdef ECMD_JABBER_SUPPORT
      if (strncmp_P(uip_appdata, PSTR("<mess"), 5) == 0)
      {
        char *body = strstr_P(uip_appdata, PSTR("<body>"));

        if (!body || jabber_extract_from())
        {
          JABDEBUG("received invalid message.\n");
          break;                /* Ignore, not really fatal. */
        }

        body += 6;              /* skip body tag. */

        char *ptr = strstr_P(uip_appdata, PSTR("</bod"));
        if (!ptr)
        {
          JABDEBUG("received incomplete message, buffer overrun?\n");
          break;
        }
        *ptr = 0;               /* terminate body text. */
        jabber_parse_ecmd(body);
        STATE->action = JABBER_ACTION_MESSAGE;
        break;
      }
#endif /* ECMD_JABBER_SUPPORT */

      if (strstr_P(uip_appdata, PSTR("type='get'")))
      {
        JABDEBUG("type=get found\n");

        if (jabber_extract_from())
          break;
        if (jabber_extract_id())
          break;

#ifdef JABBER_LAST_SUPPORT
        char *lastptr = strstr_P(uip_appdata, PSTR("iq:last"));
        if (lastptr)
        {
          JABDEBUG("iq:last found\n");
          STATE->action = JABBER_ACTION_LAST;
          return 0;
        }
#endif /* JABBER_LAST_SUPPORT */

#ifdef JABBER_VERSION_SUPPORT
        char *versionptr = strstr_P(uip_appdata, PSTR("iq:version"));
        if (versionptr)
        {
          JABDEBUG("iq:version found\n");
          STATE->action = JABBER_ACTION_VERSION;
          return 0;
        }
#endif /* JABBER_VERSION_SUPPORT */
      }                         /* End of <iq type='get'> parser. */

      JABDEBUG("got something, but no idea how to parse it: '%s'\n",
               uip_appdata);
      break;

    default:
      JABDEBUG("stage invalid\n");
      return 1;
  }

  /* Jippie, let's enter next stage if we haven't reached connected. */
  if (STATE->stage != JABBER_CONNECTED)
    STATE->stage++;
  return 0;
}

static void
jabber_main(void)
{
  if (uip_aborted() || uip_timedout())
  {
    JABDEBUG("connection aborted\n");
    jabber_conn = NULL;
  }

  if (uip_closed())
  {
    JABDEBUG("connection closed\n");
    jabber_conn = NULL;
  }

  if (uip_connected())
  {
    JABDEBUG("new connection\n");
    STATE->stage = JABBER_OPEN_STREAM;
    STATE->sent = JABBER_INIT;

#ifdef JABBER_STARTUP_MESSAGE_SUPPORT
    strncpy_P(STATE->target, PSTR(CONF_JABBER_BUDDY), sizeof(STATE->target));
    strncpy_P(STATE->outbuf, jabber_startup_text, sizeof(STATE->outbuf));
    STATE->action = JABBER_ACTION_MESSAGE;
#endif /* JABBER_STARTUP_MESSAGE_SUPPORT */
  }

  if (uip_acked() && STATE->stage == JABBER_CONNECTED)
  {
    STATE->action = JABBER_ACTION_NONE;
    *STATE->outbuf = 0;
  }

  if (uip_newdata() && uip_len)
  {
    /* Zero-terminate */
    ((char *) uip_appdata)[uip_len] = 0;
    JABDEBUG("received data: %s\n", uip_appdata);

    if (jabber_parse())
    {
      uip_close();              /* Parse error */
      return;
    }
  }

  if (uip_rexmit())
    jabber_send_data(STATE->stage, STATE->action);

  else if ((STATE->stage > STATE->sent || STATE->stage == JABBER_CONNECTED)
           && (uip_newdata() || uip_acked() || uip_connected()))
    jabber_send_data(STATE->stage, STATE->action);
  else if (STATE->stage == JABBER_CONNECTED && uip_poll() && STATE->action)
    jabber_send_data(STATE->stage, STATE->action);

}

uint8_t
jabber_send_message(char *message)
{
  if (!jabber_conn)
    return 0;
  if (STATE->outbuf[0])
    return 0;

  /* Send message to the default buddy */
  strncpy_P(STATE->target, PSTR(CONF_JABBER_BUDDY), sizeof(STATE->target));

  strncpy(STATE->outbuf, message, sizeof(STATE->outbuf));
  STATE->outbuf[sizeof(STATE->outbuf) - 1] = 0;

  return 1;
}

void
jabber_periodic(void)
{
  if (!jabber_conn)
  {
    jabber_init();
  }
}

void
jabber_init(void)
{
  JABDEBUG("initializing client\n");

  uip_ipaddr_t ip;
  set_CONF_JABBER_IP(&ip);
  jabber_conn = uip_connect(&ip, HTONS(5222), jabber_main);

  if (!jabber_conn)
  {
    JABDEBUG("no uip_conn available.\n");
    return;
  }

#ifdef JABBER_EEPROM_SUPPORT
  eeprom_restore(jabber_username, &jabber_user, JABBER_VALUESIZE);
  eeprom_restore(jabber_password, &jabber_pass, JABBER_VALUESIZE);
  eeprom_restore(jabber_resource, &jabber_resrc, JABBER_VALUESIZE);
  eeprom_restore(jabber_hostname, &jabber_host, JABBER_VALUESIZE);
#endif
}

/*
  -- Ethersex META --

  header(services/jabber/jabber.h)
  timer(500, jabber_periodic())
  net_init(jabber_init)

  state_header(services/jabber/jabber_state.h)
  state_tcp(struct jabber_connection_state_t jabber)
*/
