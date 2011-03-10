/*----------------------------------------------------------------------------
 Copyright:      Johann Gail  mailto: gailj@gmx.de
 Author:         Johann Gail
 Remarks:        
 known Problems: none
 Version:        20.12.2010
 Description:    Send MAIL Client

 Modifications to fit Ethersex firmware:
 Copyright (C) 2010 by Johann Gail 

 Dieses Programm ist freie Software. Sie k�nnen es unter den Bedingungen der 
 GNU General Public License, wie von der Free Software Foundation ver�ffentlicht, 
 weitergeben und/oder modifizieren, entweder gem�� Version 2 der Lizenz oder 
 (nach Ihrer Option) jeder sp�teren Version. 

 Die Ver�ffentlichung dieses Programms erfolgt in der Hoffnung, 
 da� es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
 F�R EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
 Programm erhalten haben. 
 Falls nicht, schreiben Sie an die Free Software Foundation, 
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
------------------------------------------------------------------------------*/

#ifndef _SIP_H
#define _SIP_H

/* Port des SIP-Interfaces */
#define SIP_PORT 5060

struct sip_connection_state_t {
    uint8_t       state;
};

#include "protocols/uip/uip.h"
//uip_conn_t *sip_call (void);
void sip_call_init (void);
void sip_main (void);

void sip_start_ringing (void);
void sip_stop_ringing (void);

#endif /* _SIP_H */
