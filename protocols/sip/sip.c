/*
 * Copyright (c) 2009 by Johann Gail <johann.gail.gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
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


/* This is a rather basic implementation of the SIP protocoll.
 * For more details of SIP protocoll see at RFC 3261
 * http://www.ietf.org/rfc/rfc3261.txt
 */

#include "config.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sip.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/crypto/md5.h"

#ifdef DEBUG_SIP
#include "core/debug.h"
#define SIP_DEBUG(str...)      debug_printf ("sip: " str)
#define SIP_DEBUG_STOP(p, str...) *p=0;debug_printf ("sip: " str)
#else
#define SIP_DEBUG(...)         ((void) 0)
#define SIP_DEBUG_STOP(...)    ((void) 0)
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define SIPS_IDLE 0
#define SIPS_INVITE 1
#define SIPS_INVITE_AUTH 2
#define SIPS_RINGING 3
#define SIPS_CANCELING 4
#define SIPS_SPEAKING 5
#define SIPS_BYE 6

static uip_udp_conn_t *udp_sip_conn = NULL;
static uint8_t sip_state = SIPS_IDLE;
static uint8_t sip_pollcounter = 0;
static uint8_t sip_cseg_counter = 0;

static char sip_realm[32];
static char sip_nonce[32];

static const char PROGMEM SIP20[] = "SIP/2.0\r\n";
static const char PROGMEM SIP_200_OK[] = "SIP/2.0 200 OK\r\n";

static const char PROGMEM SIP_CANCEL[] = "CANCEL";
static const char PROGMEM SIP_ACK[] = "ACK";
static const char PROGMEM SIP_INVITE[] = "INVITE";
static const char PROGMEM SIP_BYE[] = "BYE";
static const char PROGMEM SIP_REGISTER[] = "REGISTER";

static const char PROGMEM SIP_HEADER_URI[] =
  "sip:" CONF_SIP_TO "@" CONF_SIP_PROXY_IP;

static const char PROGMEM SIP_HEADER[] =
  " sip:" CONF_SIP_TO "@" CONF_SIP_PROXY_IP " SIP/2.0\r\n" "Via: SIP/2.0/UDP "
  CONF_ENC_IP ":" STR(SIP_PORT) ";rport;branch=z9hG4bK1234.";
static const char PROGMEM SIP_HEADER2[] =
  "\r\nFrom: \"Ethersex\" <sip:" CONF_SIP_AUTH_USER "@" CONF_SIP_PROXY_IP
  ">;tag=pfhdc\r\n" "To: <sip:" CONF_SIP_TO "@" CONF_SIP_PROXY_IP ">\r\n"
  "Max-Forwards: 70\r\n" "Call-ID: hkmhwqdsqvsmnqd@" CONF_HOSTNAME "\r\n"
  "Contact: <sip:" CONF_SIP_FROM "@" CONF_ENC_IP ">\r\n"
  "Authorization: Digest username=\"" CONF_SIP_AUTH_USER "\"";
static const char PROGMEM SIP_REALM[] = ", realm=\"";
static const char PROGMEM SIP_NONCE[] = "\", nonce=\"";
static const char PROGMEM SIP_URI[] = "\", uri=\"";
static const char PROGMEM SIP_RESPONSE[] = "\", response=\"";
static const char PROGMEM SIP_ALGO[] = "\", algorithm=MD5";
static const char PROGMEM SIP_CSEG[] = "\r\nCSeq:";
static const char PROGMEM SIP_HEADEREND[] = "\r\n\r\n";
/*const char PROGMEM SIP_CONTENT[] = 
  "\r\nUser-Agent: Ethersex/2013/Doorbell\r\n"
  "Content-Length: 311\r\n"
  "Content-Type: application/sdp\r\n"
  "\r\n"
  "v=0\r\n"
  "o=621@192.168.178.1 0 0 IN IP4 192.168.178.90\r\n"
  "s=Session SIP/SDP\r\n"
  "c=IN IP4 192.168.178.90\r\n"
  "t=0 0\r\n"
  "m=audio 21000 RTP/AVP 9 8 0 101\r\n"
  "a=rtpmap:9 G722/8000\r\n"
  "a=rtpmap:8 PCMA/8000\r\n"
  "a=rtpmap:0 PCMU/8000\r\n"
  "a=rtpmap:101 telephone-event/8000\r\n"
  "a=fmtp:101 0-15\r\n"
  "m=video 21070 RTP/AVP 103\r\n"
  "a=rtpmap:103 h263-1998/90000\r\n";
*/

static void
MD5ToHex(md5_ctx_t * md5, char *buffer)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      unsigned int val = (md5->a[i] >> 4 * j) & 0x0f;
      buffer[8 * i + (j ^ 1)] = (val < 10) ? '0' + val : 'a' - 10 + val;
    }
  }
}

static char *
sip_insert_md5_auth(char *d, const PGM_P method, const PGM_P uri)
{
  char buffer[99];              //32 + : + 32 + : + 32 + \0

  // Form "username:realm:password"
  uint8_t len = snprintf_P(buffer, sizeof(buffer) - 1,
                           PSTR(CONF_SIP_AUTH_USER ":%s:" CONF_SIP_AUTH_PASS),
                           sip_realm);
  buffer[len] = '\0';

  SIP_DEBUG("input h1: %s\r\n", buffer);
  md5_ctx_t h1;
  md5_init(&h1);
  md5_lastBlock(&h1, buffer, len * 8);

  // Form "method:digesturi"
  len = snprintf_P(buffer, sizeof(buffer) - 1, PSTR("%S:%S"), method, uri);
  buffer[len] = '\0';

  SIP_DEBUG("input h2: %s\r\n", buffer);
  md5_ctx_t h2;
  md5_init(&h2);
  md5_lastBlock(&h2, buffer, len * 8);

  // Response h1:nonce:h2
  char *p = buffer;
  MD5ToHex(&h1, p);
  p += 32;
  *p++ = ':';
  p += snprintf(p, sizeof(buffer) - 34, sip_nonce);
  *p++ = ':';
  MD5ToHex(&h2, p);
  p += 32;
  *p = '\0';

  SIP_DEBUG("input response: %s\r\n", buffer);
  md5_ctx_t response;
  md5_init(&response);
  md5_lastBlock(&response, buffer, (p - buffer) * 8);

  MD5ToHex(&response, buffer);
  buffer[32] = 0;
  SIP_DEBUG("response: %s\r\n", buffer);

  sprintf_P(d, PSTR("%S%s%S%s%S%s%S%s%S"),
            SIP_REALM, sip_realm, SIP_NONCE, sip_nonce,
            SIP_URI, uri, SIP_RESPONSE, buffer, SIP_ALGO);
  return d;
}

static char *
sip_append_cseg_number(char *p)
{
  *p = '0' + (sip_cseg_counter % 10);
  p++;
  *p = ' ';
  p++;
  return p;
}

static void
sip_send_status_200(char *uip_appdata)
{
  char *p = uip_appdata;
  char *a = strstr_P(p, SIP20);
  p += sprintf_P(p, SIP_200_OK);
  if (a != 0)
  {
    a += (sizeof(SIP20) - 1);
    char *end = uip_appdata + uip_datalen();
    while (a < end)
    {
      *p++ = *a++;
    }
  }
  uip_udp_send(p - (char *) uip_appdata);
  SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n", p - (char *) uip_appdata,
                 (char *) uip_appdata);
}

static void
sip_send_ACK(char *uip_appdata)
{
  char *p = uip_appdata;
  p += sprintf_P(p, PSTR("%S%S"), SIP_ACK, SIP_HEADER);
  p = sip_append_cseg_number(p);
  p += sprintf_P(p, PSTR("%S%S"), SIP_HEADER2, SIP_CSEG);
  p = sip_append_cseg_number(p);
  p += sprintf_P(p, PSTR("%S%S"), SIP_ACK, SIP_HEADEREND);

  uip_udp_send(p - (char *) uip_appdata);
  SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n", p - (char *) uip_appdata,
                 (char *) uip_appdata);
}

//----------------------------------------------------------------------------
//Handling des SIP
void
sip_main()
{
  if (uip_newdata())
  {

    SIP_DEBUG("SIP UDP empfangen %s\n\r", (char *) uip_appdata);

    sip_pollcounter = 0;

    if ((((char *) uip_appdata)[0] == 'B') &&
        (((char *) uip_appdata)[1] == 'Y') &&
        (((char *) uip_appdata)[2] == 'E'))
    {

      SIP_DEBUG("received SIP BYE\n\r");
      sip_send_status_200(uip_appdata);
      sip_cseg_counter++;
      sip_state = SIPS_IDLE;
      return;
    }

    uint16_t code = atol((char *) uip_appdata + 8);
    SIP_DEBUG("received data, new code: %d\n", code);

    switch (code)
    {
        // Authentication required

      case 401:
      case 407:
        {
        char *p = uip_appdata;

        char *p1 = strstr_P(p, PSTR("realm=\"")) + 7;
        char *p2 = sip_realm;
        if (p1 != NULL)
          while (*p1 != '\"')
          {
            *p2++ = *p1++;
            if (p2 >= sip_realm + sizeof(sip_realm))
            {
              SIP_DEBUG("Buffer too small!\n");
              break;
            }
          }
        *p2 = 0;

        p1 = strstr_P(p, PSTR("nonce=\"")) + 7;
        p2 = sip_nonce;
        if (p1 != NULL)
          while (*p1 != '\"')
          {
            *p2++ = *p1++;
            if (p2 >= sip_nonce + sizeof(sip_nonce))
            {
              SIP_DEBUG("Buffer too small!\n");
              break;
            }
          }
        *p2 = 0;
        }
        sip_send_ACK(uip_appdata);
        sip_cseg_counter++;
        sip_state = SIPS_INVITE_AUTH;

        break;

        // Trying
      case 100:
        sip_state = SIPS_RINGING;
        break;

        // Ringing
      case 180:
        sip_state = SIPS_RINGING;
        break;

        // Session Progress
      case 183:
        sip_state = SIPS_RINGING;
        break;

        // OK
      case 200:
        sip_send_ACK(uip_appdata);
        if (sip_state == SIPS_RINGING)
        {
          sip_state = SIPS_SPEAKING;
          sip_cseg_counter++;
          // If the invited person takes the call, terminate it immediatelly.
          //sip_state = SIPS_BYE;
        }
        else
          sip_state = SIPS_IDLE;
        break;

        // Illegal Contact Header
        // Method mismatch between requst line and CSeg
      case 400:
        sip_state = SIPS_IDLE;
        break;

        // Not found
        // Could happen with wrong FROM: field.
      case 404:
        sip_state = SIPS_IDLE;
        break;

        // User not responding
      case 480:
        sip_state = SIPS_IDLE;
        break;

        // Call Leg/Transaction does not exist (Wrong call id at bye)
      case 481:
        sip_state = SIPS_IDLE;
        break;


        // Busy here
      case 486:
        sip_send_ACK(uip_appdata);
        sip_state = SIPS_IDLE;
        break;

        // Request terminated
      case 487:
        sip_send_ACK(uip_appdata);
        sip_state = SIPS_IDLE;
        break;

        // Internal server error
      case 500:
        sip_state = SIPS_IDLE;
        break;

        // Decline
      case 603:
        sip_send_ACK(uip_appdata);
        sip_state = SIPS_IDLE;
        break;

      default:
        sip_state = SIPS_IDLE;
        break;

    }
  }
  else if (uip_poll())
  {

    if (sip_pollcounter > 0)
    {
      sip_pollcounter--;
      // Nicht endlos versuchen, irgendwann aufgeben
      if (sip_pollcounter == 0)
      {
        sip_cseg_counter++;
        sip_state = SIPS_IDLE;
      }
    }
    //Every 1 sec repeat last UDP-Telegramm, may be lost. 
    if (sip_pollcounter % 10 == 0)
    {
      SIP_DEBUG("State: %d\r\n", sip_state);
      if (sip_state == SIPS_INVITE)
      {
        char *p = uip_appdata;

        p += sprintf_P(p, PSTR("%S%S"), SIP_INVITE, SIP_HEADER);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_HEADER2, SIP_CSEG);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_INVITE, SIP_HEADEREND);

        uip_udp_send(p - (char *) uip_appdata);
        SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n",
                       p - (char *) uip_appdata, (char *) uip_appdata);
      }
      else if (sip_state == SIPS_INVITE_AUTH)
      {
        char *p = uip_appdata;
        p += sprintf_P(p, PSTR("%S%S"), SIP_INVITE, SIP_HEADER);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, SIP_HEADER2);
        p = sip_insert_md5_auth(p, SIP_INVITE, SIP_HEADER_URI);
        p += sprintf_P(p, SIP_CSEG);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_INVITE, SIP_HEADEREND);

        uip_udp_send(p - (char *) uip_appdata);
        SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n",
                       p - (char *) uip_appdata, (char *) uip_appdata);
      }
      else if (sip_state == SIPS_CANCELING)
      {
        char *p = uip_appdata;

        p += sprintf_P(p, PSTR("%S%S"), SIP_CANCEL, SIP_HEADER);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_HEADER2, SIP_CSEG);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_CANCEL, SIP_HEADEREND);

        uip_udp_send(p - (char *) uip_appdata);
        SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n",
                       p - (char *) uip_appdata, (char *) uip_appdata);
      }
      else if (sip_state == SIPS_BYE)
      {
        char *p = uip_appdata;

        p += sprintf_P(p, PSTR("%S%S"), SIP_BYE, SIP_HEADER);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_HEADER2, SIP_CSEG);
        p = sip_append_cseg_number(p);
        p += sprintf_P(p, PSTR("%S%S"), SIP_BYE, SIP_HEADEREND);

        uip_udp_send(p - (char *) uip_appdata);
        SIP_DEBUG_STOP(p, "SIP sent %d bytes:\r\n%s\r\n",
                       p - (char *) uip_appdata, (char *) uip_appdata);
      }
    }
  }
}


//----------------------------------------------------------------------------
//SIP-Call starten
//uip_conn_t *
void
sip_call_init(void)
{
  //�ffnet eine Verbindung zu einem SIP-Proxy
  SIP_DEBUG("Open SIP UDP Port\n\r");

  uip_ipaddr_t ip;
  set_CONF_SIP_PROXY_IP(&ip);
  udp_sip_conn = uip_udp_new(&ip, HTONS(SIP_PORT), sip_main);

  if (!udp_sip_conn)
  {
    SIP_DEBUG("No Space\n\r");
    return;                     /* dammit. */
  }

  uip_udp_bind(udp_sip_conn, HTONS(SIP_PORT));
}

//----------------------------------------------------------------------------
//Ring a phone by dialing a phone number.
void
sip_start_ringing(void)
{
  if (sip_state == SIPS_IDLE)
  {
    SIP_DEBUG("--Start call");
    sip_pollcounter = 100;
    sip_state = SIPS_INVITE;
  }
}

//----------------------------------------------------------------------------
//Stop ringing if not call has not been accepted yet.
void
sip_stop_ringing(void)
{
  if ((sip_state == SIPS_RINGING))
  {
    SIP_DEBUG("--Cancel call");
    sip_pollcounter = 100;
    sip_state = SIPS_CANCELING;
  }
}


/*
  -- Ethersex META --
  header(protocols/sip/sip.h)
  startup(sip_call_init)
*/
