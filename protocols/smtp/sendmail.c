/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        20.04.2008
 Description:    Send MAIL Client

 Modifications to fit Ethersex firmware:
 Copyright (C) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 
 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
 GNU General Public License, wie von der Free Software Foundation veröffentlicht,
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
 (nach Ihrer Option) jeder späteren Version. 

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
 Programm erhalten haben. 
 Falls nicht, schreiben Sie an die Free Software Foundation, 
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
------------------------------------------------------------------------------*/

#include "config.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>

#include "sendmail.h"
#include "protocols/uip/uip.h"

#ifdef DEBUG_SENDMAIL
#  include "core/debug.h"
#  define MAIL_DEBUG(str...) debug_printf ("sendmail: " str)
#else
#  define MAIL_DEBUG(...)    ((void) 0)
#endif

#define STATE (&uip_conn->appstate.sendmail)

#define MAIL_SEND(str) do { \
  memcpy_P (uip_sappdata, str, sizeof (str));     \
  uip_send (uip_sappdata, sizeof (str) - 1);      \
} while(0)

const char PROGMEM SMTP_HELO[] = "HELO Ethersex\r\n";
const char PROGMEM SMTP_AUTH[] = "AUTH LOGIN\r\n";
const char PROGMEM SMTP_RSET[] = "RSET\r\n";
const char PROGMEM SMTP_MAIL[] = "MAIL FROM:<" CONF_SENDMAIL_FROM ">\r\n";
const char PROGMEM SMTP_RCPT[] = "RCPT TO:<" CONF_SENDMAIL_TO ">\r\n";
const char PROGMEM SMTP_DATA[] = "DATA\r\n";
const char PROGMEM SMTP_TEXT[] =
  "From: \"Ethersex\" <" CONF_SENDMAIL_FROM ">\r\n"
  "Subject: Test-Mail!"
  "\r\n\r\n"
  /* The text to send follows below, feel free to adjust it to your needs. */

  "Hallo, Ethersex aktiv!"

  /* don't touch the end-of-message mark! Just edit the text above. */
  "\r\n.\r\n";
const char PROGMEM SMTP_QUIT[] = "QUIT\r\n";

void
sendmail_net_main (void)
{
  if (uip_aborted () || uip_timedout ())
    {
      if (STATE->retries)
        {
	  /* trigger another one and copy retries count. */
	  uip_conn_t *conn = mail_send ();
	  conn->appstate.sendmail.retries = STATE->retries - 1;
	}
      return;
    }

  if (uip_closed ())
    return;

  MAIL_DEBUG ("sendmail_net_main called.\n");

  if (uip_acked ()) {
    STATE->state ++;
    MAIL_DEBUG ("received ack, setting state to %d.\n", STATE->state);
  }

  if (uip_newdata ()) {
    STATE->code = atol ((char *) uip_appdata);
    MAIL_DEBUG ("received data, new code: %d\n", STATE->code);

#ifdef SENDMAIL_AUTH_SUPPORT
    /* Fehler 503 (authentication not enabled) abfangen und sofort ohne AUTH senden */
    if (STATE->code == 503 && STATE->state == 2)	
      {
	STATE->state = 4;
	STATE->code = 235;
      }
#endif  /* SENDMAIL_AUTH_SUPPORT */
  }

  if (STATE->code >= 500 || STATE->code == 451)
    {
      MAIL_DEBUG ("\r\n\r\n*** Error: Mail wurde nicht versendet ***\r\n");
      MAIL_DEBUG ("(Message Code: %i)\r\n", STATE->code);
      uip_close ();
      return;
    }

  MAIL_DEBUG ("generating data, state=%d, code=%d\n", 
              STATE->state, STATE->code);


  unsigned char mail_auth_str[35];

  switch (STATE->state)
    {
    case 0:
      if (STATE->code == 220)
        {
          MAIL_DEBUG ("0: SMTP_HELO\n");
	  MAIL_SEND (SMTP_HELO);
        }
      break;

    case 1:
#ifdef SENDMAIL_AUTH_SUPPORT
      if (STATE->code == 250)
        {
          MAIL_DEBUG ("1: SMTP_AUTH\n");
	  MAIL_SEND (SMTP_AUTH);
        }
      break;

    case 2:

      if (STATE->code == 334)
        {
          uint8_t len = strlen(CONF_SENDMAIL_USERNAME_BASE64);
          MAIL_DEBUG ("2: Send Username: %s\n", CONF_SENDMAIL_USERNAME_BASE64);
          memcpy_P (uip_sappdata, PSTR(CONF_SENDMAIL_USERNAME_BASE64 "\r\n"), 
                  len + 2); 
          uip_send (uip_sappdata, len + 3);
        }
      break;

    case 3:
      if (STATE->code == 334)
        {
          uint8_t len = strlen(CONF_SENDMAIL_PASSWORD_BASE64);
          MAIL_DEBUG ("3: Send Password: %s\n", CONF_SENDMAIL_PASSWORD_BASE64);
          memcpy_P (uip_sappdata, PSTR(CONF_SENDMAIL_PASSWORD_BASE64 "\r\n"), 
                  len + 2); 
          uip_send (uip_sappdata, len + 3);
        }
      break;

#else  /* not SENDMAIL_AUTH_SUPPORT */
      STATE->state += 3;
#endif

    case 4:
      if (STATE->code == 235 || STATE->code == 250)
        {
          MAIL_DEBUG ("4: SMTP_MAIL_FROM\n\r");
	  MAIL_SEND (SMTP_MAIL);
        }
      break;

    case 5:
      if (STATE->code == 250)
        {
          MAIL_DEBUG ("5: SMTP_MAIL_RCPT_TO\n\r");
	  MAIL_SEND (SMTP_RCPT);
        }
      break;

    case 6:
      if (STATE->code == 250)
        {
          MAIL_DEBUG ("6: SMTP_MAIL_DATA\n\r");
	  MAIL_SEND (SMTP_DATA);
        }
      break;

    case 7:
      if (STATE->code == 354)
        {
          MAIL_DEBUG ("7: SMTP_MAIL_TEXT\n\r");
	  MAIL_SEND (SMTP_TEXT);
        }
      break;

    case 8:
      if (STATE->code == 250)
        {
          MAIL_DEBUG ("8: SMTP_MAIL_QUIT\n\r");
	  MAIL_SEND (SMTP_QUIT);
        }
      break;

    case 9:
      MAIL_DEBUG ("9: Mail wurde versendet!!\r\n");
      uip_close ();
    }
}

//----------------------------------------------------------------------------
//Versenden einer E-MAIL starten
uip_conn_t *
mail_send (void)
{
  //öffnet eine Verbindung zu einem EMAIL-Server
  MAIL_DEBUG ("Send E-Mail (Sie haben Post ;-)\n\r");

  uip_ipaddr_t ip;
  set_CONF_SENDMAIL_IP(&ip);

  uip_conn_t *conn = uip_connect (&ip, HTONS (MAIL_PORT), sendmail_net_main);
  if (! conn) return NULL;

  conn->appstate.sendmail.state = 0;
  conn->appstate.sendmail.code = 0;
  conn->appstate.sendmail.retries = 2;

  return conn;
}

/*
  -- Ethersex META --
  header(protocols/smtp/sendmail.h)
  startup(mail_send)

  state_header(protocols/smtp/sendmail.h)
  state_tcp(struct sendmail_connection_state_t sendmail)
*/
