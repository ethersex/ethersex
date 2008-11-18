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

#include "../config.h"

#ifdef SENDMAIL_SUPPORT

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>

#include "sendmail.h"

#ifdef DEBUG_SENDMAIL
#  include "../debug.h"
#  define MAIL_DEBUG debug_printf
#else
#  define MAIL_DEBUG(...)  ((void) 0)
#endif

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
  "Hallo, Ethersex aktiv!";
const char PROGMEM SMTP_END[] = "\r\n.\r\n";
const char PROGMEM SMTP_QUIT[] = "QUIT\r\n";

unsigned char mail_enable = 0;

//----------------------------------------------------------------------------
//Daten kommen vom EMAIL- Server an!! (Mail wird versendet)
void
mail_data (unsigned char index)
{
  //Verbindung wurde abgebaut!
  if (tcp_entry[index].status & FIN_FLAG)
    {
      return;
    }

  unsigned char mail_auth_str[35];
  unsigned int message_code = 0;

  message_code = atol ((char *) &eth_buffer[TCP_DATA_START_VAR]);

  if (message_code == 503 && mail_send_counter == 2)	//Fehler 503 (authentication not enabled) abfangen und sofort ohne AUTH senden
    {
      mail_send_counter = 4;
      message_code = 235;
    }

  if (message_code >= 500 || message_code == 451)
    {
      MAIL_DEBUG
        ("\r\n\r\n*** Error: Mail wurde nicht versendet ***\r\n");
      MAIL_DEBUG ("(Message Code: %i)\r\n", message_code);
      tcp_entry[index].time = TCP_TIME_OFF;
      return;
    }

  if (message_code != 0)
    {
      for (int a = TCP_DATA_START_VAR; a < TCP_DATA_END_VAR; a++)
        {
          MAIL_DEBUG ("%c", eth_buffer[a]);
        }
    }

  switch (mail_send_counter)
    {
    case 0:
      if (message_code == 220)
        {
          MAIL_DEBUG ("0: SMTP_HELO\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_HELO,
    		sizeof (SMTP_HELO));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_HELO) - 1, index);
          mail_send_counter++;
        }
      break;

    case 1:
      if (message_code == 250)
        {
          MAIL_DEBUG ("1: SMTP_AUTH\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_AUTH,
    		sizeof (SMTP_AUTH));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_AUTH) - 1, index);
          mail_send_counter++;
        }
      break;

    case 2:
      if (message_code == 334)
        {
          MAIL_DEBUG ("2: Send Username: ");
          decode_base64 ((unsigned char *) SMTP_USERNAME, mail_auth_str);
          MAIL_DEBUG ("%s", mail_auth_str);
          MAIL_DEBUG ("\r\n");

          memcpy (&eth_buffer[TCP_DATA_START_VAR], mail_auth_str,
    	      sizeof (mail_auth_str));
          memcpy_P (&eth_buffer
    		[TCP_DATA_START_VAR + sizeof (mail_auth_str) - 1],
    		PSTR ("\r\n"), 2);

          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (mail_auth_str) + 1, index);
          mail_send_counter++;
        }
      break;

    case 3:
      if (message_code == 334)
        {
          MAIL_DEBUG ("3: Send Password: ");
          decode_base64 ((unsigned char *) SMTP_PASSWORD, mail_auth_str);
          MAIL_DEBUG ("%s", mail_auth_str);
          MAIL_DEBUG ("\r\n");

          memcpy (&eth_buffer[TCP_DATA_START_VAR], mail_auth_str,
    	      sizeof (mail_auth_str));
          memcpy_P (&eth_buffer
    		[TCP_DATA_START_VAR + sizeof (mail_auth_str) - 1],
    		PSTR ("\r\n"), 2);


          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (mail_auth_str) + 1, index);
          mail_send_counter++;
        }
      break;

    case 4:
      if (message_code == 235)
        {
          MAIL_DEBUG ("4: SMTP_MAIL_FROM\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_MAIL,
    		sizeof (SMTP_MAIL));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_MAIL) - 1, index);
          mail_send_counter++;
        }
      break;

    case 5:
      if (message_code == 250)
        {
          MAIL_DEBUG ("5: SMTP_MAIL_RCPT_TO\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_RCPT,
    		sizeof (SMTP_RCPT));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_RCPT) - 1, index);
          mail_send_counter++;
        }
      break;

    case 6:
      if (message_code == 250)
        {
          MAIL_DEBUG ("6: SMTP_MAIL_DATA\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_DATA,
    		sizeof (SMTP_DATA));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_DATA) - 1, index);
          mail_send_counter++;
        }
      break;

    case 7:
      if (message_code == 354)
        {
          MAIL_DEBUG ("7: SMTP_MAIL_TEXT\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_TEXT,
    		sizeof (SMTP_TEXT));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_TEXT) - 1, index);
          mail_send_counter++;
        }
      break;

    case 8:
      MAIL_DEBUG ("8: SMTP_MAIL_END\n\r");
      memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_END,
    	    sizeof (SMTP_END));
      tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
      create_new_tcp_packet (sizeof (SMTP_END) - 1, index);
      mail_send_counter++;
      break;

    case 9:
      if (message_code == 250)
        {
          MAIL_DEBUG ("9: SMTP_MAIL_QUIT\n\r");
          memcpy_P (&eth_buffer[TCP_DATA_START_VAR], SMTP_QUIT,
    		sizeof (SMTP_QUIT));
          tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
          create_new_tcp_packet (sizeof (SMTP_QUIT) - 1, index);
          mail_send_counter++;
        }
      break;

    case 10:
      MAIL_DEBUG ("\r\n10: Mail wurde versendet!!\r\n");

    }
}

//----------------------------------------------------------------------------
//Versenden einer E-MAIL starten
void
mail_send (void)
{
  //öffnet eine Verbindung zu einem EMAIL-Server
  MAIL_DEBUG ("Send E-Mail (Sie haben Post ;-)\n\r");

  //ARP Request senden
  if (arp_request (MAIL_SERVER))
    {
      mail_send_counter = 0;

      unsigned long index = MAX_ARP_ENTRY;
      unsigned char tmp_counter = 0;
      while ((index >= MAX_ARP_ENTRY)
    	 && (tcp_entry[index].app_status != 1))
        {
          index = tcp_entry_search (MAIL_SERVER, LBBL_ENDIAN_INT (MAIL_PORT));
          if (tmp_counter++ > 30)
	    {
	      MAIL_DEBUG ("TCP Eintrag nicht gefunden (Mailserver)\r\n");
	      return;
	    }
        }
      MAIL_DEBUG ("TCP Eintrag gefunden (Mailserver)!\r\n");
    }
}

#endif  /* SENDMAIL_SUPPORT */

