/*
 * Copyright (c) 2009 by Stefan Müller <mueller@cos-gbr.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include <ctype.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#include "core/eeprom.h"
#include "sms77.h"

static char *sms77_tmp_buf;

static const char PROGMEM sms77_secheader[] =
    " HTTP/1.1\n"
    "Host: " CONF_SMS77_SERVICE "\n\n";


int 
urlencode(char* src, int nb, char* dst)
{
	int nb_w = 0;
	for(; nb; src++, nb--)
	{
		if(isalnum(*src))
		{
			*dst++ = *src;
			nb_w++;	
		}		
		else
		{
			sprintf(dst,"%%%2.x", *src);
			nb_w += 3;
			dst += 3;
		}	
	}
	return nb_w;
}

static void
sms77_net_main(void)
{	
	
    if (uip_aborted() || uip_timedout()) {
	SMSDEBUG ("connection aborted\n");
        if (sms77_tmp_buf) {
          free(sms77_tmp_buf);
          sms77_tmp_buf = NULL;
        }
        return;
    }

    if (uip_closed()) {
	SMSDEBUG ("connection closed\n");
        if (sms77_tmp_buf) {
          free(sms77_tmp_buf);
          sms77_tmp_buf = NULL;
        }
        return;
    }


    if (uip_connected() || uip_rexmit()) {
	SMSDEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;
        p += sprintf(p,  "GET /?u=%s&p=%s&to=%s&type=%s&text=", sms77_user, sms77_pass, sms77_recv, sms77_type);
        p += urlencode(sms77_tmp_buf, strlen(sms77_tmp_buf), p);
        p += sprintf_P(p, sms77_secheader);
        uip_udp_send(p - (char *)uip_appdata);
        SMSDEBUG("send %d bytes\n", p - (char *)uip_appdata);
    }

    if (uip_acked()) {
      uip_close();
    }
    
    if (uip_newdata())
    	SMSDEBUG("data: %s\n", uip_appdata);

}

static void
sms77_dns_query_cb(char *name, uip_ipaddr_t *ipaddr) {
  SMSDEBUG("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), sms77_net_main)) {
    if (sms77_tmp_buf) {
      free(sms77_tmp_buf);
      sms77_tmp_buf = NULL;
    }
  }

}

uint8_t
sms77_send(char *status)
{
  /* Transmission taking action */
  if (sms77_tmp_buf) return 0;

  uint8_t len = strlen(status);
  if (len > 160) {
    SMSDEBUG("message too long: cropping");
    len = 160;
  }

  sms77_tmp_buf = malloc(160);
  if (!sms77_tmp_buf) return 0;

  memcpy(sms77_tmp_buf, status, len);
  sms77_tmp_buf[len] = 0;

  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_SMS77_SERVICE))) {
    resolv_query(CONF_SMS77_SERVICE, sms77_dns_query_cb);
  } else {
    sms77_dns_query_cb(NULL, ipaddr);
  }
  return 1;
}

void
sms77_init(void)
{
#ifdef SMS77_EEPROM_SUPPORT
	eeprom_restore(sms77_username, &sms77_user, SMS77_VALUESIZE);
	eeprom_restore(sms77_password, &sms77_pass, SMS77_VALUESIZE);
	eeprom_restore(sms77_receiver, &sms77_recv, SMS77_VALUESIZE);
	eeprom_restore(sms77_type, &sms77_type,  SMS77_VALUESIZE);
#else
	sprintf(sms77_user, "%s", CONF_SMS77_USER);
	sprintf(sms77_pass, "%s", CONF_SMS77_PASS);
	sprintf(sms77_recv, "%s", CONF_SMS77_TO);
	sprintf(sms77_type, "%s", CONF_SMS77_TYPE);
#endif	
}



/*
  -- Ethersex META --
  init(sms77_init)
*/
