/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        20.04.2008
 Description:    Send MAIL Client

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

#include <config.h>

#if USE_MAIL
#ifndef _SENDMAIL_H
#define _SENDMAIL_H

#include <avr/io.h>
#include <string.h>
#include "usart.h"
#include "base64.h"
unsigned char mail_enable;
unsigned int mail_send_counter;
unsigned int my_mail_cp;	//cp = CLIENT_PORT

  //Port des EMAIL-SERVERS
#define MAIL_PORT 25

  //Mit oder ohne Debug-Ausgabe
#define MAIL_DEBUG usart_write	//mit Debugausgabe
  //#define MAIL_DEBUG(...)                   //ohne Debugausgabe

  //IP des MAIL-Servers 
#define MAIL_SERVER	IP(212,227,15,183)	//hier z.B. smtp.1und1.de
  //#define MAIL_SERVER       IP(69,147,102,58) //hier z.B. smtp.mail.yahoo.com
  //#define MAIL_SERVER       IP(213,165,64,20) //hier z.B. mail.gmx.net

  //Hier Username eintragen
#define SMTP_USERNAME "tester\0"

  //Hier das Passwort eintragen
#define SMTP_PASSWORD "tester\0"

  //SMTP_MAIL Sender
#define __SMTP_MAIL_FROM__ "mail@ulrichradig.de"

  //SMTP_MAIL Empfänger
#define __SMTP_MAIL_TO__ "mail@ulrichradig.de"
unsigned char mail_send (void);
void mail_client_init (void);
void mail_data (unsigned char);

#endif //_SENDMAIL_H
#endif //USE_MAIL
