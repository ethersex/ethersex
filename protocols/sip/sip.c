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
#  include "core/debug.h"
#  define SIP_DEBUG(str...) debug_printf ("sip: " str)
#else
#  define SIP_DEBUG(...)    ((void) 0)
#endif

#define my_strcat_P(p, s) { strcpy_P(p, s);  p+= strlen_P(s);}
//#define my_strcat_P(p, s) { strcpy_P(p, s);  p+= sizeof(s)-1;}	//Flash doesn't change, so can take constants instead of function calls


#define SIPS_IDLE 0
#define SIPS_INVITE 1 
#define SIPS_INVITE_AUTH 2
#define SIPS_RINGING 3
#define SIPS_CANCEL 4
#define SIPS_BYE 5

uip_udp_conn_t *udp_sip_conn = NULL;
uint8_t state = SIPS_IDLE;
uint8_t pollcounter = 90;
uint8_t cseg_counter = 0;

char realm[32];
char nonce[32];


const char PROGMEM SIP_CANCEL[] = "CANCEL";
const char PROGMEM SIP_ACK[]    = "ACK";
const char PROGMEM SIP_INVITE[] = "INVITE"; 
const char PROGMEM SIP_HEADER[] = " sip:"CONF_SIP_TO"@" CONF_SIP_PROXY_IP " SIP/2.0\r\n"
																	"Via: SIP/2.0/UDP "CONF_SIP_PROXY_IP";branch=z9hG4bK12345678\r\n"
                                  "From: \"Doorbell\" <sip:"CONF_SIP_AUTH_USER"@"CONF_SIP_PROXY_IP">;tag=pfhdc\r\n"
																	"To: <sip:"CONF_SIP_TO"@"CONF_SIP_PROXY_IP">\r\n"
																	"Max-Forwards: 70\r\n"
																	"Call-ID: hkmhwqdsqvsmnqd@"CONF_HOSTNAME"\r\n"
																	"Contact: <sip:"CONF_SIP_FROM"@"CONF_HOSTNAME">\r\n"
																	"Authorization: Digest username=\""CONF_SIP_AUTH_USER"\"";
const char PROGMEM SIP_REALM[]	= ",realm=\"";																	
const char PROGMEM SIP_NONCE[]	= "\",nonce=\"";																	
const char PROGMEM SIP_RESPONSE[]	= "\",uri=\"sip:"CONF_SIP_TO"@"CONF_SIP_PROXY_IP"\",response=\"";
const char PROGMEM SIP_ALGO[]	  = "\",algorithm=MD5";
const char PROGMEM SIP_CSEG[]	  = "\r\nCSeq: 350 ";
const char PROGMEM SIP_HEADEREND[] =	"\r\n\r\n";

void 
MD5ToHex(md5_ctx_t *md5, char* buffer)
{
  int i,j;
	for (i = 0; i < 4;  i++)
	{
		for (j = 0; j < 8;  j++) 
		{
			unsigned int val = (md5->a[i] >> 4*j) & 0x0f;
		  buffer[8*i+(j^1)] = (val<10) ? '0'+val : 'a'-10+val;
		}
	}
}

/*
void
md5(void* block, uint16_t length)
{
	md5_ctx_t ctx_state;
	md5_init(&ctx_state);
	md5_lastBlock(&ctx_state, block, length);
}
*/

void 
sip_send_ACK(char* uip_appdata) {
	char* p = uip_appdata;
  my_strcat_P(p, SIP_ACK);
  my_strcat_P(p, SIP_HEADER);
  my_strcat_P(p, SIP_CSEG);
  my_strcat_P(p, SIP_ACK);
  my_strcat_P(p, SIP_HEADEREND);

  uip_udp_send(p - (char *)uip_appdata);
  SIP_DEBUG ("SIP sent %d %d %s\n\r", strlen(p), p - (char *)uip_appdata, (char *)uip_appdata );
}

//----------------------------------------------------------------------------
//Handling des SIP
void 
sip_main()
{
  if (uip_newdata ()) 
  {

    SIP_DEBUG ("SIP UDP empfangen %s\n\r", (char *) uip_appdata);
    
    pollcounter = 0;
    
    uint16_t code = atol ((char *) uip_appdata + 8);
    SIP_DEBUG ("received data, new code: %d\n", code);

		switch (code) {
    // Authentication required
    
    	case 401:
			case 407:
				{ 
	      char *p = uip_appdata;
     
  	    char *p1 = strstr_P(p, PSTR("realm=\""))+7; 
    	  char *p2 = realm;
				if (p1 != NULL)
	      	while (*p1 != '\"') {	//TODO Bufferoverflow possible!
  	    		*p2++ = *p1++;
	    	  }
  	    *p2 = 0;
      
				p1 = strstr_P(p, PSTR("nonce=\""))+7; 
      	p2 = nonce;
				if (p1 != NULL)
		      while (*p1 != '\"') {	//TODO Bufferoverflow possible!
  		    	*p2++ = *p1++;
    		  }
      	*p2 = 0;
      	}
				sip_send_ACK(uip_appdata);
				state = SIPS_INVITE_AUTH;
				
				break;
    
    // Trying
    case 100:
			state = SIPS_RINGING;
			break;
			
    // Ringing
    case 180:
			state = SIPS_RINGING;
			state = SIPS_CANCEL;
			break;

    // Session Progress
    case 183: 
    	break;
    	
    // OK
    case 200: 
			sip_send_ACK(uip_appdata);
			//state = SIPS_SPEAKING;
			state = SIPS_BYE;
    	break;

    // Illegal Contact Header
    // Method mismatch between requst line and CSeg
    case 400:
			state = SIPS_IDLE;
    	break;
    	
    // Not found
    // Could happen with wrong FROM: field.
    case 404:
			state = SIPS_IDLE;
    	break;
    	
    // User not responding
    case 480:
			state = SIPS_IDLE;
    	break;
    	
    // Busy here
    case 486:
			state = SIPS_IDLE;
    	break;
    
    // Request terminated
    case 487:
			sip_send_ACK(uip_appdata);
			state = SIPS_IDLE;
    	break;

    // Decline
    case 603:
			sip_send_ACK(uip_appdata);
			state = SIPS_IDLE;
    	break;
    }
	}
	else if (uip_poll()) {

   	pollcounter++;
   	// Nicht endlos versuchen, irgendwann aufgeben
   	if (pollcounter > 100) {
   		pollcounter = 0;
   		cseg_counter++;
   		state = SIPS_IDLE;
   	}    		
 		//Every 1 sec repeat last UDP-Telegramm, may be lost. 
   	if (pollcounter % 20 == 1) {
	    if (state == SIPS_INVITE) {  
	      char *p = uip_appdata;

	      my_strcat_P(p, SIP_INVITE);
	      my_strcat_P(p, SIP_HEADER);
			  my_strcat_P(p, SIP_CSEG);
  			my_strcat_P(p, SIP_INVITE);
	      my_strcat_P(p, SIP_HEADEREND);
	  
	      uip_udp_send(p - (char *)uip_appdata);
	      SIP_DEBUG ("SIP sent %d %d %s\n\r", strlen(p), p - (char *)uip_appdata, (char *)uip_appdata );
	    }
	    else if (state == SIPS_INVITE_AUTH) 
	    {  
	      // Form "username:realm:password"
	      char buffer[99]; //32 + : + 32 + : + 32 + \0
	      char* p = buffer;
	      strcpy(p, CONF_SIP_AUTH_USER);
	      p+=strlen(CONF_SIP_AUTH_USER);
	      *p++ = ':';
	      strcpy(p, realm);
	      p+=strlen(realm);
	      *p++ = ':';
	      strcpy(p, CONF_SIP_AUTH_PASS);
	      p+=strlen(CONF_SIP_AUTH_PASS);
	      *p = 0;
	      
	      SIP_DEBUG("input h1: %s\r\n", buffer);
		    md5_ctx_t h1;
		    md5_init(&h1);
		    md5_lastBlock(&h1, buffer, (p-buffer)*8);
	
	
	      // Form "method:digesturi"
	      p = buffer;
	      strcpy(p, "INVITE");
	      p+=strlen("INVITE");
	      *p++ = ':';
	      strcpy(p, "sip:"CONF_SIP_TO"@"CONF_SIP_PROXY_IP);
	      p+=strlen("sip:"CONF_SIP_TO"@"CONF_SIP_PROXY_IP);
	      *p = 0;
	
	      SIP_DEBUG("input h2: %s\r\n", buffer);
		    md5_ctx_t h2;
		    md5_init(&h2);
		    md5_lastBlock(&h2, buffer, (p-buffer)*8);
	
				// Response h1:nonce:h2
	      p = buffer;
				MD5ToHex(&h1, p);
				p += 32;
				*p++ = ':';
	      strcpy(p, nonce);
	      p+=strlen(nonce);
	      *p++ = ':';
				MD5ToHex(&h2, p);
				p += 32;
	      *p = 0;
				
	     	SIP_DEBUG("input response: %s\r\n", buffer);
		    md5_ctx_t response;
		    md5_init(&response);
		    md5_lastBlock(&response, buffer, (p-buffer)*8);
	
				MD5ToHex(&response, buffer);
				buffer[32]=0;
	     	SIP_DEBUG("response: %s\r\n", buffer);
	
	
	      p = uip_appdata;
	      my_strcat_P(p, SIP_INVITE);
	      my_strcat_P(p, SIP_HEADER);
	      my_strcat_P(p, SIP_REALM);
	      strcpy(p, realm);
	      p+=strlen(realm);
	      my_strcat_P(p, SIP_NONCE);
	      strcpy(p, nonce);
	      p+=strlen(nonce);
	      my_strcat_P(p, SIP_RESPONSE);
	      strcpy(p, buffer);
	      p+=strlen(buffer);
	      my_strcat_P(p, SIP_ALGO);
			  my_strcat_P(p, SIP_CSEG);
  			my_strcat_P(p, SIP_INVITE);
	      my_strcat_P(p, SIP_HEADEREND);
	
		    uip_udp_send(p - (char *)uip_appdata);
	      SIP_DEBUG ("SIP sent %d %d %s\n\r", strlen(p), p - (char *)uip_appdata, (char *)uip_appdata );
	    }
	    else if (state == SIPS_CANCEL) {  
	      char *p = uip_appdata;
	
	      my_strcat_P(p, SIP_CANCEL);
	      my_strcat_P(p, SIP_HEADER);
			  my_strcat_P(p, SIP_CSEG);
  			my_strcat_P(p, SIP_CANCEL);
	      my_strcat_P(p, SIP_HEADEREND);
	  
	      uip_udp_send(p - (char *)uip_appdata);
	      SIP_DEBUG ("SIP sent %d %d %s\n\r", strlen(p), p - (char *)uip_appdata, (char *)uip_appdata );
	    }
	  }
	}
}


//----------------------------------------------------------------------------
//SIP-Call starten
//uip_conn_t *
void
sip_call_init (void)
{
  //�ffnet eine Verbindung zu einem SIP-Proxy
  SIP_DEBUG ("Open SIP UDP Port\n\r");

  uip_ipaddr_t ip;
  set_CONF_SIP_PROXY_IP(&ip);
  udp_sip_conn = uip_udp_new (&ip, HTONS (SIP_PORT), sip_main);

  if (!udp_sip_conn) {
    SIP_DEBUG ("No Space\n\r");
	  return; /* dammit. */
  }
	
	uip_udp_bind(udp_sip_conn, HTONS (SIP_PORT));
}

//----------------------------------------------------------------------------
//Ring a phone by dialing a phone number.
void 
sip_start_ringing(void)
{
	if (state == SIPS_IDLE) {
			SIP_DEBUG("--Start call");
			pollcounter = 0;
   		state = SIPS_INVITE; 
   	}
}

//----------------------------------------------------------------------------
//Stop ringing if not call has not been accepted yet.
void 
sip_stop_ringing(void)
{
	if ((state == SIPS_RINGING)) {
		SIP_DEBUG("--Cancel call");
		pollcounter = 0;
   	state = SIPS_CANCEL; 
  }
}


/*
  -- Ethersex META --
  header(protocols/sip/sip.h)
  startup(sip_call_init)
*/
