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

#ifndef _SENDMAIL_H
#define _SENDMAIL_H

/* Port des EMAIL-SERVERS */
#define MAIL_PORT 25

struct sendmail_connection_state_t {
    uint8_t       retries;
    uint8_t       state;
    uint16_t	  code;
};

#include "protocols/uip/uip.h"
uip_conn_t *mail_send (void);

#endif /* _SENDMAIL_H */
