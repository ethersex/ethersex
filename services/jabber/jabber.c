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
#include "version.h"
#include "protocols/uip/uip.h"
#include "core/eeprom.h"
#include "jabber.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "known_buddies.c"

#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
#include "core/util/base64.h"
static void jabber_parse_sasl_challenge(const char *challenge_data);
static void jabber_build_sasl_digest_response(char *response_buf,
                                              uint16_t buf_len);
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
#include "core/util/base64.h"
extern void sha1(void *dest, const void *msg, uint32_t bitlen);
static void scram_handle_server_first(const char *data);
static void __attribute__((unused)) scram_build_client_first(char *out, uint16_t out_len);
static void scram_build_client_final(char *out, uint16_t out_len);
#endif /* JABBER_AUTH_SCRAM_SHA1 */


#ifdef JABBER_EEPROM_SUPPORT
static const char PROGMEM jabber_stream_text[] =
  "<?xml version='1.0'?>"
  "<stream:stream xmlns:stream='http://etherx.jabber.org/streams' "
  "xmlns='jabber:client' to='%s' " "from='" CONF_HOSTNAME "' xml:lang='en' >";

static const char PROGMEM jabber_get_auth_text[] __attribute__((unused)) =
  "<iq id='ga' type='get'><query xmlns='jabber:iq:auth'>"
  "<username>%s</username></query></iq>";

static const char PROGMEM jabber_set_auth_text[] __attribute__((unused)) =
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

static const char PROGMEM jabber_get_auth_text[] __attribute__((unused)) =
  "<iq id='ga' type='get'><query xmlns='jabber:iq:auth'>"
  "<username>" CONF_JABBER_USERNAME "</username></query></iq>";

static const char PROGMEM jabber_set_auth_text[] __attribute__((unused)) =
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

static const char PROGMEM jabber_startup_text[] __attribute__((unused)) =
  /* This message must NOT be longer than STATE->outbuf,
   * be careful ;) */
  "Your Ethersex '" CONF_HOSTNAME "' is now UP :)";


#define JABBER_SEND_BUFLEN (sizeof(UIP_BUFSIZE)-UIP_IPTCPH_LEN-UIP_LLH_LEN-1)
/* Syslog flushes debug lines at 100 bytes and drops entries once the queue
 * is full, so stanzas are capped; without syslog they are logged fully. */
#ifdef DEBUG_USE_SYSLOG
#define JAB_DEBUG_STANZA_MAX 56
#else
#define JAB_DEBUG_STANZA_MAX 1024
#endif
#define JABBER_SEND(...) {                                           \
    int len;                                                         \
    len = snprintf_P(uip_sappdata, JABBER_SEND_BUFLEN, __VA_ARGS__); \
    JABDEBUG("send[%d]:%.*s\n", len, JAB_DEBUG_STANZA_MAX,           \
             (((char *)uip_sappdata)[len] = 0, uip_sappdata));       \
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
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      JABBER_SEND(PSTR("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
                       "mechanism='DIGEST-MD5'/>"));
      STATE->sasl_state = JABBER_SASL_STATE_INIT;
#elif JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      {
        char client_first[128];
        char client_first_b64[180];
        scram_build_client_first(client_first, sizeof(client_first));
        base64_encode((uint8_t*)client_first, strlen(client_first),
                      client_first_b64, sizeof(client_first_b64));
        JABDEBUG("Sending SCRAM-SHA-1 client-first: %s\n", client_first);
        JABBER_SEND(PSTR("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
                         "mechanism='SCRAM-SHA-1'>%s</auth>"),
                    client_first_b64);
        STATE->scram_state = 1;
      }
#else
      JABBER_SEND_E(jabber_get_auth_text, jabber_user);
#endif
      break;

    case JABBER_SET_AUTH:
#if JABBER_AUTH_METHOD == JABBER_AUTH_PLAIN
      JABBER_SEND_E(jabber_set_auth_text, jabber_resrc, jabber_user,
                    jabber_pass);
#else
      /* Should not reach here with SASL, auth should complete via SASL */
      return;
#endif
      break;

    case JABBER_SASL_AUTH:
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      if (STATE->sasl_state == JABBER_SASL_STATE_RSPAUTH_RECEIVED)
      {
        /* Second step per RFC 2831: answer server's rspauth with empty response */
        JABBER_SEND(PSTR("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>"));
        /* Next we expect <success/> */
        STATE->sasl_state = JABBER_SASL_STATE_RESPONSE_SENT;
        break;
      }
      {
        char response[512];
        jabber_build_sasl_digest_response(response, sizeof(response));
        JABBER_SEND(PSTR("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
                         ">%s</response>"),
                     response);
        STATE->sasl_state = JABBER_SASL_STATE_RESPONSE_SENT;
      }
      break;
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      {
        char client_final[180];
        char client_final_b64[240];
        scram_build_client_final(client_final, sizeof(client_final));
        base64_encode((uint8_t*)client_final, strlen(client_final),
                      client_final_b64, sizeof(client_final_b64));
        JABBER_SEND(PSTR("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>%s</response>"),
                    client_final_b64);
        STATE->scram_state = 2;
      }
      break;
#endif /* JABBER_AUTH_SCRAM_SHA1 */

    case JABBER_SEND_BIND:
      JABBER_SEND(PSTR("<iq type='set' id='bind1'>"
                       "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>"
                       "</iq>"));
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
            JABBER_SEND(PSTR("<message to='%s' type='chat'>"
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
  size_t buddies = sizeof(jabber_known_buddies) / sizeof(jabber_known_buddies[0]);
  for (size_t i = 0; i < buddies; ++i)
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

  char *e = strstr(uip_appdata, "<stream:error");
  if (e)
  {
    char *c = strchr(e, '>');
    if (c)
    {
      while (*++c == ' ' || *c == '\t' || *c == '\r' || *c == '\n')
        ;
      if (*c == '<')
      {
        char *n = ++c;
        while (*c && *c != ' ' && *c != '/' && *c != '>')
          c++;
        *c = 0;
        JABDEBUG("STREAM ERROR condition: %s", n);
      }
    }
    return 1;
  }

  switch (STATE->stage)
  {
    case JABBER_OPEN_STREAM:
      if (strstr_P(uip_appdata, PSTR("<stream:stream")) == NULL)
      {
        JABDEBUG("<stream:stream not found in reply.  stop.");
        return 1;
      }
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      /* Second stream after SASL success: bind a resource before presence.
       * Must return directly, the trailing stage++ would skip the bind. */
      if (STATE->sasl_complete)
      {
        JABDEBUG("second stream, SASL done, going to bind\n");
        STATE->stage = JABBER_SEND_BIND;
        return 0;
      }
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      if (STATE->scram_complete)
      {
        JABDEBUG("second stream, SCRAM done, going to bind\n");
        STATE->stage = JABBER_SEND_BIND;
        return 0;
      }
#endif /* JABBER_AUTH_SCRAM_SHA1 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      /* Check if server advertises SASL DIGEST-MD5 support in stream features */
      if (strstr_P(uip_appdata, PSTR("DIGEST-MD5")))
      {
        /* We'll send SASL auth after GET_AUTH stage */
      }
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      if (strstr_P(uip_appdata, PSTR("SCRAM-SHA-1")))
      {
        JABDEBUG("Server supports SCRAM-SHA-1 SASL\n");
      }
#endif /* JABBER_AUTH_SCRAM_SHA1 */
      break;
    case JABBER_GET_AUTH:
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      if (strstr_P(uip_appdata, PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        char *challenge_ptr = strstr_P(uip_appdata,
                      PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"));
        if (challenge_ptr)
        {
          char *data_start = strchr(challenge_ptr, '>');
          if (data_start)
          {
            data_start++;
            char *data_end = strchr(data_start, '<');
            if (data_end)
            {
              *data_end = 0;
              jabber_parse_sasl_challenge(data_start);
              STATE->sasl_state = JABBER_SASL_STATE_CHALLENGE_RECEIVED;
            }
          }
        }
        /* Return directly: the trailing stage++ must not run,
         * it would skip SASL_AUTH and send presence instead. */
        STATE->stage = JABBER_SASL_AUTH;
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SASL authentication successful (no challenge)\n");
        /* XMPP requires a new stream after SASL success. Reopen it
         * instead of sending presence on the authenticated stream. */
        STATE->sasl_complete = 1;
        STATE->stage = JABBER_OPEN_STREAM;
        STATE->sent = JABBER_INIT;
        jabber_send_data(JABBER_OPEN_STREAM, STATE->action);
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SASL authentication failed\n");
        return 1;
      }
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      if (strstr_P(uip_appdata, PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SCRAM challenge received\n");
        char *challenge_ptr = strstr_P(uip_appdata,
                      PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"));
        if (challenge_ptr)
        {
          char *data_start = strchr(challenge_ptr, '>');
          if (data_start)
          {
            data_start++;
            char *data_end = strchr(data_start, '<');
            if (data_end)
            {
              *data_end = 0;
              JABDEBUG("SCRAM challenge[%d]:%.*s\n", (int)strlen(data_start),
                       JAB_DEBUG_STANZA_MAX, data_start);
              uint8_t decoded[128];
              base64_decode(data_start, decoded, sizeof(decoded));
              scram_handle_server_first((char*)decoded);
              STATE->scram_state = 1;
            }
          }
        }
        /* Return directly, see DIGEST-MD5 above. */
        STATE->stage = JABBER_SASL_AUTH;
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SCRAM authentication successful (no challenge)\n");
        STATE->scram_complete = 1;
        STATE->stage = JABBER_OPEN_STREAM;
        STATE->sent = JABBER_INIT;
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SCRAM authentication failed\n");
        return 1;
      }
#endif /* JABBER_AUTH_SCRAM_SHA1 */

      if (strstr_P(uip_appdata, PSTR("<password/>")) == NULL)
      {
        JABDEBUG("<password/> not found in reply.  stop.");
        return 1;
      }
      break;

    case JABBER_SASL_AUTH:
#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
      /* In SASL_AUTH stage, we've sent our DIGEST response and are waiting
       * for success/failure (or the rspauth second step per RFC 2831). */
      if (strstr_P(uip_appdata, PSTR("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SASL DIGEST authentication successful\n");
        /* XMPP requires a new stream after SASL success (RFC 3920 4.3.3).
         * Send the restarted stream header right here: the generic send gate
         * in jabber_poll() only fires on new data / acked / connected, and
         * after <success/> none of those hold, so the restart header would
         * never be sent and the server drops the session with
         * not-authorized. */
        STATE->sasl_complete = 1;
        STATE->stage = JABBER_OPEN_STREAM;
        STATE->sent = JABBER_INIT;
        jabber_send_data(JABBER_OPEN_STREAM, STATE->action);
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SASL DIGEST authentication failed\n");
        return 1;
      }
      /* Second step per RFC 2831: server challenge containing rspauth.
       * Answer with an empty response and stay in SASL_AUTH. */
      if (strstr_P(uip_appdata, PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        char *cptr = strstr_P(uip_appdata,
                      PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"));
        if (cptr)
        {
          char *ds = strchr(cptr, '>');
          if (ds)
          {
            uint8_t rsp[128];
            char *de;
            ds++;
            de = strchr(ds, '<');
            if (de)
            {
              *de = 0;
              base64_decode(ds, rsp, sizeof(rsp));
              if (strstr_P((char *)rsp, PSTR("rspauth")))
              {
                JABDEBUG("SASL rspauth received, sending empty response\n");
                STATE->sasl_state = JABBER_SASL_STATE_RSPAUTH_RECEIVED;
                /* Force retransmission of this stage with new content */
                STATE->sent = JABBER_SET_AUTH;
                return 0;
      }
            }
          }
        }
        JABDEBUG("Unexpected SASL challenge in SASL_AUTH stage\n");
        return 1;
      }
      JABDEBUG("Unexpected data in SASL_AUTH stage\n");
      return 1;
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
      /* SCRAM-SHA-1: waiting for server-final success */
      if (strstr_P(uip_appdata, PSTR("<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SCRAM authentication successful\n");
        /* Optional: verify server signature in success data */
        STATE->scram_state = 2;
        STATE->scram_complete = 1;
        STATE->stage = JABBER_OPEN_STREAM;
        STATE->sent = JABBER_INIT;
        jabber_send_data(JABBER_OPEN_STREAM, STATE->action);
        return 0;
      }
      if (strstr_P(uip_appdata, PSTR("<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("SCRAM authentication failed\n");
        return 1;
      }
      if (strstr_P(uip_appdata, PSTR("<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'")))
      {
        JABDEBUG("Unexpected SCRAM challenge in SASL_AUTH stage\n");
        return 1;
      }
      JABDEBUG("Unexpected data in SCRAM SASL_AUTH stage\n");
      return 1;
#endif /* JABBER_AUTH_SCRAM_SHA1 */

    case JABBER_SET_AUTH:
      if (strstr_P(uip_appdata, PSTR("result")) == NULL)
      {
        JABDEBUG("authentication failed.  stop.");
        return 1;
      }

      JABDEBUG("jippie, we successfully authenticated to the server\n");
      break;

    case JABBER_SEND_BIND:
      if (strstr_P(uip_appdata, PSTR("<stream:features")))
        return 0;

      if (strstr_P(uip_appdata, PSTR("type='result'")) == NULL)
      {
        JABDEBUG("resource bind failed.  stop.");
        return 1;
      }
      JABDEBUG("resource bound\n");
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

      JABDEBUG("unparsed[%d]:%.*s\n", uip_len, JAB_DEBUG_STANZA_MAX,
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

#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
    STATE->sasl_state = JABBER_SASL_STATE_INIT;
    STATE->sasl_nc = 0;
    STATE->sasl_complete = 0;
    STATE->sasl_nonce[0] = 0;
    STATE->sasl_realm[0] = 0;
    STATE->sasl_qop[0] = 0;
    STATE->sasl_algorithm[0] = 0;
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
    STATE->scram_state = 0;
    STATE->scram_complete = 0;
    STATE->scram_client_nonce[0] = 0;
    STATE->scram_server_nonce[0] = 0;
    STATE->scram_salt_len = 0;
    STATE->scram_iteration_count = 4096;
#endif /* JABBER_AUTH_SCRAM_SHA1 */

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
    JABDEBUG("recv[%d]:%.*s\n", uip_len, JAB_DEBUG_STANZA_MAX, uip_appdata);

    if (jabber_parse())
    {
      JABDEBUG("PARSE ERR at stage=%d -> closing\n", STATE->stage);
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
  /* Don't try before the stack has an address - ENC28J60 link and
   * configuration (static or DHCP) are not yet ready at
   * ethersex_meta_netinit time. Periodic will retry. */
  {
    uip_ipaddr_t host, zero;
    uip_gethostaddr(&host);
    memset(&zero, 0, sizeof(zero));
    if (uip_ipaddr_cmp(&host, &zero))
      return;
  }

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

#if JABBER_AUTH_METHOD == JABBER_AUTH_DIGEST_MD5
#include "core/crypto/md5.h"
#include "core/util/byte2hex.h"
#include "core/util/base64.h"

static const char PROGMEM jabber_sasl_uri_format[] = "xmpp/%s";
static const char PROGMEM jabber_sasl_qop_default[] = "auth";
static const char PROGMEM jabber_sasl_response_format[] =
  "username=\"%s\",realm=\"%s\",nonce=\"%s\",nc=%s,cnonce=\"%s\","
  "qop=%s,digest-uri=\"%s\",response=%s,charset=utf-8";

/* Parse SASL DIGEST-MD5 challenge and extract parameters */
/* Challenge format per RFC 2831: base64(realm="...",nonce="...",qop="...",...)*/
static void
jabber_parse_sasl_challenge(const char *challenge_data)
{
  uint8_t decoded[256];
  char *ptr;


  /* Decode base64 challenge; base64_decode NUL-terminates the output */
  base64_decode((char *) challenge_data, decoded, sizeof(decoded));

  /* Parse key=value pairs per RFC 2831. Values may be quoted-string
   * or token (unquoted). Commas inside quoted-string must not split. */
  ptr = (char *)decoded;
  while (ptr && *ptr)
  {
    /* Skip whitespace and commas between pairs */
    while (*ptr == ' ' || *ptr == ',') ptr++;

    if (!*ptr) break;

    /* Find key */
    char *key = ptr;
    while (*ptr && *ptr != '=') ptr++;

    if (!*ptr) break;

    *ptr++ = 0; /* Terminate key */

    /* Trim whitespace around key (RFC allows SP) */
    /* key already without trailing SP because we stopped at '=', but
     * caller may have leading SP already skipped. */

    char *value = NULL;
    char *value_end = NULL;

    if (*ptr == '"')
    {
      ptr++; /* Skip opening quote */
      value = ptr;
      while (*ptr && *ptr != '"') ptr++;
      value_end = ptr;
      if (*ptr == '"')
        *ptr++ = 0; /* Terminate value, skip closing quote */

      /* Store parameter based on key */
      if (strcmp(key, "nonce") == 0)
      {
        strncpy(STATE->sasl_nonce, value, sizeof(STATE->sasl_nonce) - 1);
        STATE->sasl_nonce[sizeof(STATE->sasl_nonce) - 1] = 0;
      }
      else if (strcmp(key, "realm") == 0)
      {
        strncpy(STATE->sasl_realm, value, sizeof(STATE->sasl_realm) - 1);
        STATE->sasl_realm[sizeof(STATE->sasl_realm) - 1] = 0;
      }
      else if (strcmp(key, "qop") == 0)
      {
        /* qop may be list like "auth,auth-int" - pick first token */
        char *comma = strchr(value, ',');
        if (comma) *comma = 0;
        strncpy(STATE->sasl_qop, value, sizeof(STATE->sasl_qop) - 1);
        STATE->sasl_qop[sizeof(STATE->sasl_qop) - 1] = 0;
      }
      else if (strcmp(key, "algorithm") == 0)
      {
        strncpy(STATE->sasl_algorithm, value, sizeof(STATE->sasl_algorithm) - 1);
        STATE->sasl_algorithm[sizeof(STATE->sasl_algorithm) - 1] = 0;
      }
      else if (strcmp(key, "charset") == 0)
      {
        /* ignore, we always use utf-8 */
      }
      else
      {
      }
      (void)value_end;
    }
    else
    {
      /* Unquoted value (token) - skip to comma or end, trim trailing SP */
      value = ptr;
      while (*ptr && *ptr != ',' ) ptr++;
      /* Trim trailing whitespace */
      char *end = ptr;
      while (end > value && (end[-1] == ' ' || end[-1] == '\t')) end--;
      *end = 0;
      /* Advance ptr over comma will be done by outer loop; ensure NUL */
      if (*ptr == ',') {
        *ptr++ = 0;
        /* ptr already advanced, but we terminated value at end */
      } else if (*ptr) {
        ptr++;
      }

      if (strcmp(key, "algorithm") == 0)
      {
        strncpy(STATE->sasl_algorithm, value, sizeof(STATE->sasl_algorithm) - 1);
        STATE->sasl_algorithm[sizeof(STATE->sasl_algorithm) - 1] = 0;
      }
      else if (strcmp(key, "qop") == 0)
      {
        char *comma = strchr(value, ',');
        if (comma) *comma = 0;
        strncpy(STATE->sasl_qop, value, sizeof(STATE->sasl_qop) - 1);
        STATE->sasl_qop[sizeof(STATE->sasl_qop) - 1] = 0;
      }
      else if (strcmp(key, "nonce") == 0)
      {
        strncpy(STATE->sasl_nonce, value, sizeof(STATE->sasl_nonce) - 1);
        STATE->sasl_nonce[sizeof(STATE->sasl_nonce) - 1] = 0;
      }
      else if (strcmp(key, "realm") == 0)
      {
        strncpy(STATE->sasl_realm, value, sizeof(STATE->sasl_realm) - 1);
        STATE->sasl_realm[sizeof(STATE->sasl_realm) - 1] = 0;
      }
    }
  }
}

/* Compute the hex representation of MD5(data) and store 32 chars + NUL in dest. */
static void
jabber_md5_hex(const void *data, uint16_t len, char *dest)
{
  static const char hex_digits[] PROGMEM = "0123456789abcdef";
  md5_hash_t hash;
  uint8_t i;

  md5(&hash, data, (uint32_t) len * 8);
  for (i = 0; i < MD5_HASH_BYTES; i++)
  {
    dest[i * 2] = pgm_read_byte(&hex_digits[(hash[i] >> 4) & 0xF]);
    dest[i * 2 + 1] = pgm_read_byte(&hex_digits[hash[i] & 0xF]);
  }
  dest[MD5_HASH_BYTES * 2] = 0;
}

/* Build the SASL DIGEST-MD5 auth response per RFC 2831 / RFC 3920.
   The RFC 2831 response string is base64-encoded into response_buf.
   The raw response string is assembled in uip_sappdata, which is
   overwritten by the caller when sending the final message. */
static void
jabber_build_sasl_digest_response(char *response_buf, uint16_t buf_len)
{
  char user[32], pass[32], host[64];
  char realm[JABBER_SASL_MAX_PARAM_LEN], nonce[JABBER_SASL_MAX_PARAM_LEN];
  char qop[8], nc_str[9], cnonce[17], uri[80];
  char ha1_hex[33], ha2_hex[33], response_hex[33];
  md5_hash_t ha1_bin, ha2_bin;

  /* Resolve credentials - either from EEPROM globals or compile-time config */
#ifdef JABBER_EEPROM_SUPPORT
  strncpy(user, jabber_user, sizeof(user) - 1);
  strncpy(pass, jabber_pass, sizeof(pass) - 1);
  strncpy(host, jabber_host, sizeof(host) - 1);
#else
  strncpy_P(user, PSTR(CONF_JABBER_USERNAME), sizeof(user) - 1);
  strncpy_P(pass, PSTR(CONF_JABBER_PASSWORD), sizeof(pass) - 1);
  strncpy_P(host, PSTR(CONF_JABBER_HOSTNAME), sizeof(host) - 1);
#endif
  user[sizeof(user) - 1] = 0;
  pass[sizeof(pass) - 1] = 0;
  host[sizeof(host) - 1] = 0;

  /* Realm and nonce from the server challenge, with sensible fallbacks */
  strncpy(realm, STATE->sasl_realm[0] ? STATE->sasl_realm : host,
          sizeof(realm) - 1);
  realm[sizeof(realm) - 1] = 0;
  strncpy(nonce, STATE->sasl_nonce, sizeof(nonce) - 1);
  nonce[sizeof(nonce) - 1] = 0;

  /* qop: use server offer if present, else default to "auth" */
  if (STATE->sasl_qop[0])
  {
    strncpy(qop, STATE->sasl_qop, sizeof(qop) - 1);
    qop[sizeof(qop) - 1] = 0;
  }
  else
  {
    strncpy_P(qop, jabber_sasl_qop_default, sizeof(qop) - 1);
    qop[sizeof(qop) - 1] = 0;
  }

  /* Nonce count and client nonce (first 16 hex chars of MD5(user:nc)) */
  STATE->sasl_nc++;
  snprintf(nc_str, sizeof(nc_str), "%08x", STATE->sasl_nc);

  {
    char cnonce_input[48];
    char cnonce_full[33];
    snprintf(cnonce_input, sizeof(cnonce_input), "%s:%s", user, nc_str);
    jabber_md5_hex(cnonce_input, strlen(cnonce_input), cnonce_full);
    memcpy(cnonce, cnonce_full, 16);
    cnonce[16] = 0;
  }

  /* HA1 = MD5(user:realm:pass), with md5-sess: MD5(MD5(user:realm:pass):nonce:cnonce) */
  {
    char a1_str[96];
    snprintf(a1_str, sizeof(a1_str), "%s:%s:%s", user, realm, pass);
    md5(&ha1_bin, a1_str, (uint32_t) strlen(a1_str) * 8);

    if (strncmp(STATE->sasl_algorithm, "md5-sess", 8) == 0
        || STATE->sasl_algorithm[0] == 0)
    {
      /* RFC 2831 md5-sess: H(A1) where A1 = MD5(user:realm:pass):nonce:cnonce
       * ha1_bin currently holds MD5(user:realm:pass) binary (16 bytes).
       * The nonce goes in exactly as the server sent it, i.e. the quoted
       * base64 directive value, NOT the base64-decoded bytes.  jabberd2/Cyrus
       * hashes the transmitted string here; using the raw bytes produces a
       * valid-looking but rejected response.  Verified against the live
       * server: text nonce -> success, decoded bytes -> not-authorized. */
      uint8_t a1_sess[16 + 1 + JABBER_SASL_MAX_PARAM_LEN + 1 + 16];
      uint8_t *p = a1_sess;
      memcpy(p, ha1_bin, MD5_HASH_BYTES);
      p += MD5_HASH_BYTES;
      *p++ = ':';
      memcpy(p, nonce, strlen(nonce));
      p += strlen(nonce);
      *p++ = ':';
      memcpy(p, cnonce, strlen(cnonce));
      p += strlen(cnonce);
      uint16_t a1_sess_len = p - a1_sess;
      md5(&ha1_bin, a1_sess, (uint32_t) a1_sess_len * 8);
    }
    /* hex representation for KD */
    for (uint8_t i = 0; i < MD5_HASH_BYTES; i++)
    {
      static const char hex_digits[] PROGMEM = "0123456789abcdef";
      ha1_hex[i * 2] = pgm_read_byte(&hex_digits[(ha1_bin[i] >> 4) & 0xF]);
      ha1_hex[i * 2 + 1] = pgm_read_byte(&hex_digits[ha1_bin[i] & 0xF]);
    }
    ha1_hex[32] = 0;
  }

  /* digest-uri = xmpp/host, HA2 = MD5(AUTHENTICATE:digest-uri) */
  snprintf_P(uri, sizeof(uri), jabber_sasl_uri_format, host);
  {
    char a2_str[128];
    snprintf(a2_str, sizeof(a2_str), "AUTHENTICATE:%s", uri);
    md5(&ha2_bin, a2_str, (uint32_t) strlen(a2_str) * 8);
    for (uint8_t i = 0; i < MD5_HASH_BYTES; i++)
    {
      static const char hex_digits[] PROGMEM = "0123456789abcdef";
      ha2_hex[i * 2] = pgm_read_byte(&hex_digits[(ha2_bin[i] >> 4) & 0xF]);
      ha2_hex[i * 2 + 1] = pgm_read_byte(&hex_digits[ha2_bin[i] & 0xF]);
    }
    ha2_hex[32] = 0;
  }

  /* response = MD5(HA1_hex:nonce:nc:cnonce:qop:HA2_hex) */
  {
    char kd_input[160];
    /* HA1_hex (32) + ":" + nonce + ":" + nc (8) + ":" + cnonce (16) + ":" + qop + ":" + HA2_hex (32) */
    snprintf(kd_input, sizeof(kd_input), "%s:%s:%s:%s:%s:%s",
             ha1_hex, nonce, nc_str, cnonce, qop, ha2_hex);
    jabber_md5_hex(kd_input, strlen(kd_input), response_hex);
  }

  /* Assemble the RFC 2831 response string in uip_sappdata and base64 it */
  snprintf_P(uip_sappdata, JABBER_SEND_BUFLEN, jabber_sasl_response_format,
             user, realm, nonce, nc_str, cnonce, qop, uri, response_hex);
  base64_encode((const uint8_t *) uip_sappdata,
                strlen((char *) uip_sappdata), response_buf, buf_len);
}
#endif /* JABBER_AUTH_DIGEST_MD5 */
#if JABBER_AUTH_METHOD == JABBER_AUTH_SCRAM_SHA1
/* SCRAM-SHA-1 implementation (RFC 5802) without channel binding. */

#define SCRAM_SHA1_HASH_BYTES 20
#define SCRAM_SHA1_BLOCK_BYTES 64

static void scram_hmac_sha1(const uint8_t *key, uint8_t key_len,
                            const uint8_t *data, uint16_t data_len,
                            uint8_t *out)
{
  uint8_t k_ipad[SCRAM_SHA1_BLOCK_BYTES];
  uint8_t k_opad[SCRAM_SHA1_BLOCK_BYTES];
  uint8_t tk[SCRAM_SHA1_HASH_BYTES];
  uint8_t i;

  if (key_len > SCRAM_SHA1_BLOCK_BYTES) {
    sha1(tk, key, (uint32_t)key_len * 8);
    key = tk;
    key_len = SCRAM_SHA1_HASH_BYTES;
  }
  memset(k_ipad, 0x36, sizeof(k_ipad));
  memset(k_opad, 0x5c, sizeof(k_opad));
  for (i = 0; i < key_len; i++) {
    k_ipad[i] ^= key[i];
    k_opad[i] ^= key[i];
  }
  /* inner = SHA1(k_ipad || data) */
  uint8_t inner[SCRAM_SHA1_HASH_BYTES];
  /* Use a temporary buffer for inner hash: k_ipad (64) + data (<=256) */
  uint8_t buf[SCRAM_SHA1_BLOCK_BYTES + 128];
  memcpy(buf, k_ipad, SCRAM_SHA1_BLOCK_BYTES);
  memcpy(buf + SCRAM_SHA1_BLOCK_BYTES, data, data_len);
  sha1(inner, buf, (uint32_t)(SCRAM_SHA1_BLOCK_BYTES + data_len) * 8);
  /* outer = SHA1(k_opad || inner) */
  memcpy(buf, k_opad, SCRAM_SHA1_BLOCK_BYTES);
  memcpy(buf + SCRAM_SHA1_BLOCK_BYTES, inner, SCRAM_SHA1_HASH_BYTES);
  sha1(out, buf, (uint32_t)(SCRAM_SHA1_BLOCK_BYTES + SCRAM_SHA1_HASH_BYTES) * 8);
}

static void scram_hi(const uint8_t *str, uint8_t str_len,
                     const uint8_t *salt, uint8_t salt_len,
                     uint32_t iterations, uint8_t *out)
{
  uint8_t u[SCRAM_SHA1_HASH_BYTES];
  uint8_t tmp[SCRAM_SHA1_HASH_BYTES];
  uint8_t salt_int[36];
  uint32_t j, k;

  memcpy(salt_int, salt, salt_len);
  salt_int[salt_len]     = 0;
  salt_int[salt_len + 1] = 0;
  salt_int[salt_len + 2] = 0;
  salt_int[salt_len + 3] = 1;
  scram_hmac_sha1(str, str_len, salt_int, salt_len + 4, u);
  memcpy(out, u, SCRAM_SHA1_HASH_BYTES);
  for (j = 1; j < iterations; j++) {
    scram_hmac_sha1(str, str_len, u, SCRAM_SHA1_HASH_BYTES, tmp);
    memcpy(u, tmp, SCRAM_SHA1_HASH_BYTES);
    for (k = 0; k < SCRAM_SHA1_HASH_BYTES; k++)
      out[k] ^= u[k];
  }
}

static void __attribute__((unused)) scram_build_client_first(char *out, uint16_t out_len)
{
  char user[32], cnonce[24];
  char cnonce_input[32];
  uint8_t i;

  /* Use same cnonce derivation as DIGEST for determinism without extra RNG */
#ifdef JABBER_EEPROM_SUPPORT
  strncpy(user, jabber_user, sizeof(user) - 1);
#else
  strncpy_P(user, PSTR(CONF_JABBER_USERNAME), sizeof(user) - 1);
#endif
  user[sizeof(user) - 1] = 0;
  snprintf(cnonce_input, sizeof(cnonce_input), "%s:%u", user, (unsigned)STATE->scram_client_nonce[0]);
  /* Simple cnonce: hex of sha1 is overkill, use base64-like random from md5 */
  for (i = 0; i < 16; i++)
    cnonce[i] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ (user[i % strlen(user)] + i * 7) & 0x3F ];
  cnonce[16] = 0;
  strncpy(STATE->scram_client_nonce, cnonce, sizeof(STATE->scram_client_nonce) - 1);
  STATE->scram_client_nonce[sizeof(STATE->scram_client_nonce) - 1] = 0;
  snprintf_P(out, out_len, PSTR("n,,n=%s,r=%s"), user, cnonce);
}

static void scram_handle_server_first(const char *data)
{
  char *p, *end;
  uint8_t salt_dec[32];
  uint16_t salt_len = 0;
  uint32_t iter = 4096;
  char combined[64];

  /* data is base64 decoded server-first-message */
  /* Expected: r=client+server,s=base64(salt),i=4096 */
  p = strstr(data, "r=");
  if (p) {
    p += 2;
    end = strchr(p, ',');
    if (end) *end = 0;
    strncpy(STATE->scram_server_nonce, p, sizeof(STATE->scram_server_nonce) - 1);
    STATE->scram_server_nonce[sizeof(STATE->scram_server_nonce) - 1] = 0;
    if (end) *end = ',';
    /* Verify server nonce starts with client nonce */
    if (strncmp(STATE->scram_server_nonce, STATE->scram_client_nonce, strlen(STATE->scram_client_nonce)) != 0) {
      JABDEBUG("SCRAM server nonce does not start with client nonce\n");
    }
    strncpy(combined, p, sizeof(combined) - 1);
    combined[sizeof(combined) - 1] = 0;
  }
  p = strstr(data, "s=");
  if (p) {
    p += 2;
    end = strchr(p, ',');
    if (end) *end = 0;
    {
      uint16_t b64_len = strlen(p);
      uint8_t pad = 0;
      if (b64_len && p[b64_len - 1] == '=') pad++;
      if (b64_len > 1 && p[b64_len - 2] == '=') pad++;
      salt_len = (b64_len * 3) / 4 - pad;
      base64_decode(p, salt_dec, sizeof(salt_dec));
    }
    STATE->scram_salt_len = salt_len;
    if (salt_len > sizeof(STATE->scram_salt)) salt_len = sizeof(STATE->scram_salt);
    memcpy(STATE->scram_salt, salt_dec, salt_len);
    if (end) *end = ',';
  }
  p = strstr(data, "i=");
  if (p) {
    p += 2;
    iter = atol(p);
    if (iter == 0) iter = 4096;
    STATE->scram_iteration_count = iter;
  }
}

static void scram_build_client_final(char *out, uint16_t out_len)
{
  char user[32], pass[32];
  uint8_t salted[SCRAM_SHA1_HASH_BYTES];
  uint8_t client_key[SCRAM_SHA1_HASH_BYTES];
  uint8_t stored_key[SCRAM_SHA1_HASH_BYTES];
  uint8_t client_sig[SCRAM_SHA1_HASH_BYTES];
  uint8_t proof[SCRAM_SHA1_HASH_BYTES];
  char proof_b64[32];
  char client_final_wo_proof[96];
  uint8_t i;

#ifdef JABBER_EEPROM_SUPPORT
  strncpy(user, jabber_user, sizeof(user) - 1);
  strncpy(pass, jabber_pass, sizeof(pass) - 1);
#else
  strncpy_P(user, PSTR(CONF_JABBER_USERNAME), sizeof(user) - 1);
  strncpy_P(pass, PSTR(CONF_JABBER_PASSWORD), sizeof(pass) - 1);
#endif
  user[sizeof(user) - 1] = 0;
  pass[sizeof(pass) - 1] = 0;

  scram_hi((uint8_t*)pass, strlen(pass), STATE->scram_salt, STATE->scram_salt_len, STATE->scram_iteration_count, salted);
  scram_hmac_sha1(salted, SCRAM_SHA1_HASH_BYTES, (uint8_t*)"Client Key", 10, client_key);
  sha1(stored_key, client_key, (uint32_t)SCRAM_SHA1_HASH_BYTES * 8);
  snprintf(client_final_wo_proof, sizeof(client_final_wo_proof), "c=biws,r=%s", STATE->scram_server_nonce);
  {
    char salt_b64[32];
    char server_first[128];
    char client_first_bare[64];
    char auth_message[256];
    base64_encode(STATE->scram_salt, STATE->scram_salt_len, salt_b64, sizeof(salt_b64));
    snprintf(server_first, sizeof(server_first), "r=%s,s=%s,i=%lu", STATE->scram_server_nonce, salt_b64, (unsigned long)STATE->scram_iteration_count);
    snprintf(client_first_bare, sizeof(client_first_bare), "n=%s,r=%s", user, STATE->scram_client_nonce);
    snprintf(auth_message, sizeof(auth_message), "%s,%s,%s", client_first_bare, server_first, client_final_wo_proof);
    scram_hmac_sha1(stored_key, SCRAM_SHA1_HASH_BYTES, (uint8_t*)auth_message, strlen(auth_message), client_sig);
  }
  for (i = 0; i < SCRAM_SHA1_HASH_BYTES; i++)
    proof[i] = client_key[i] ^ client_sig[i];
  base64_encode(proof, SCRAM_SHA1_HASH_BYTES, proof_b64, sizeof(proof_b64));
  snprintf_P(out, out_len, PSTR("%s,p=%s"), client_final_wo_proof, proof_b64);
}
#endif /* JABBER_AUTH_SCRAM_SHA1 */

/*
  -- Ethersex META --

  header(services/jabber/jabber.h)
  timer(500, jabber_periodic())
  net_init(jabber_init)

  state_header(services/jabber/jabber_state.h)
  state_tcp(struct jabber_connection_state_t jabber)
*/
