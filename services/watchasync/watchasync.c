/*
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Jens Wilmer <ethersex@jenswilmer.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

/*
 * watchasync.c
 * 
 * based heavily on httplog.c based heavily on twitter.c
 * purpose:
 * Watch changes on PORTC via Interrupt and log httplog like against a web server
 * optionally including the current unix time stamp and a unique machine identifier (uuid)
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#include "core/portio/portio.h"
#include "protocols/ecmd/sender/ecmd_sender_net.h"
#include "watchasync.h"

#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
#include "services/clock/clock.h"
#endif

#define WATCHASYNC_BUFFERSIZE 64

// first string is the GET part including the path
static const char PROGMEM get_string_head[] =
    "GET " CONF_WATCHASYNC_PATH "?port=";
// next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_WATCHASYNC_INCLUDE_PREFIX
static const char PROGMEM prefix_string[] =
	CONF_WATCHASYNC_PREFIX ;
#endif
#ifdef CONF_WATCHASYNC_INCLUDE_UUID
static const char PROGMEM uuid_string[] =
	"&uuid=" CONF_WATCHASYNC_UUID ;
#endif
// the - optional - unix time stamp
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
static const char PROGMEM time_string[] =
	"&time=";
#endif
// and the http footer including the http protocol version and the server name
static const char PROGMEM get_string_foot[] =
    " HTTP/1.1\r\n"
    "Host: " CONF_WATCHASYNC_SERVICE "\r\n\r\n";


static struct WatchAsyncBuffer wa_buffer[WATCHASYNC_BUFFERSIZE]; // Ringbuffer for Messages

static uint8_t wa_buffer_left = 0; 	// last position sent
static uint8_t wa_buffer_right = 0; 	// last position set

static uint8_t wa_portstate = 0; 		// Last portstate saved
static uint8_t wa_sendstate = 0; 		// 0: Idle, 1: Message being sent, 2: Sending message failed

void watchcat_edge(uint8_t pin);

// Handle Interrupts
ISR(PCINT2_vect)
{
  uint8_t portcompstate = (PINC ^ wa_portstate);
  while (portcompstate)
  {
    for (uint8_t pin = 0; pin < 8; pin ++)
    {
      if (portcompstate & wa_portstate & (1 << pin)) // bit changed from 1 to 0
      {
        wa_buffer_right = ((wa_buffer_right + 1) % WATCHASYNC_BUFFERSIZE);
	if (wa_buffer_right == wa_buffer_left)
	{
	  wa_buffer_right = ((wa_buffer_right - 1) % WATCHASYNC_BUFFERSIZE);
	  WATCHASYNC_DEBUG ("Buffer full, discarding message!");
	} else {
	  wa_buffer[wa_buffer_right].pin = pin;
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
          wa_buffer[wa_buffer_right].timestamp = clock_get_time();
#endif
	}
      }
    }
    wa_portstate ^= portcompstate;
    portcompstate = (PINC ^ wa_portstate);
  }
}

static void watchasync_net_main(void)
{
  if (uip_aborted() || uip_timedout()) 
  {
    WATCHASYNC_DEBUG ("connection aborted\n");
    wa_sendstate = 2;
    return;
  }

  if (uip_closed()) 
  {
    WATCHASYNC_DEBUG ("connection closed\n");
    wa_sendstate = 0;
    return;
  }


  if (uip_connected() || uip_rexmit()) {
    WATCHASYNC_DEBUG ("new connection or rexmit, sending message\n");
    char *p = uip_appdata;
    p += sprintf_P(p, get_string_head);
#ifdef CONF_WATCHASYNC_INCLUDE_PREFIX
    p += sprintf_P(p, prefix_string);
#endif
    p += sprintf(p, "%u", wa_buffer[wa_buffer_left].pin);
#ifdef CONF_WATCHASYNC_INCLUDE_UUID
    p += sprintf_P(p, uuid_string);
#endif
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
    p += sprintf_P(p, time_string);
    p += sprintf(p, "%lu", wa_buffer[wa_buffer_left].timestamp);
#endif
    p += sprintf_P(p, get_string_foot);
    uip_udp_send(p - (char *)uip_appdata);
    WATCHASYNC_DEBUG ("send %d bytes\n", p - (char *)uip_appdata);
    WATCHASYNC_DEBUG ("send %s \n", uip_appdata);
  }

  if (uip_acked()) {
    uip_close();
    WATCHASYNC_DEBUG ("packet sent, closing\n");
  }
}


static void watchasync_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)
{
  WATCHASYNC_DEBUG ("got dns response, connecting\n");
  if(!uip_connect(ipaddr, HTONS(80), watchasync_net_main)) 
  {
    wa_sendstate = 2;
  }
}

void sendmessage(void)
{
  wa_sendstate = 1; // set new state

  uip_ipaddr_t *ipaddr;
  if (!(ipaddr = resolv_lookup(CONF_WATCHASYNC_SERVICE))) { // Try to find IPAddress
    resolv_query(CONF_WATCHASYNC_SERVICE, watchasync_dns_query_cb); // If not found: query DNS
  } else {
    watchasync_dns_query_cb(NULL, ipaddr); // If found use IPAddress
  }
  return;
}

void watchasync_init(void)
{
  PORTC = (1<<PB7)|(1<<PB6)|(1<<PB5)|(1<<PB4)|(1<<PB3)|(1<<PB2)|(1<<PB1)|(1<<PB0);  // Enable Pull-up on PortC
  DDRC = 0; 			// PortC Input
  wa_portstate = PINC; 		// save current state
  PCMSK2 = (1<<PCINT23)|(1<<PCINT22)|(1<<PCINT21)|(1<<PCINT20)|(1<<PCINT19)|(1<<PCINT18)|(1<<PCINT17)|(1<<PCINT16);  // Enable Pinchange Interrupt on PortC
  PCICR |= 1<<PCIE2;		// Enable Pinchange Interrupt on PortC
//  SREG |= 1<<I;			//Enable Interrupts (will hopefully be done somewhere else)
}

void watchasync_mainloop(void)
{
  if (wa_sendstate != 1) // not busy sending 
  {
    if (wa_sendstate == 2) // Message not sent successfully
    {
      WATCHASYNC_DEBUG ("Error, again please...\n"); 
      sendmessage();
    } else // sendstate == 0 => Idle
    {
      if (wa_buffer_left != wa_buffer_right) // there is somethiing in the buffer
      {
        wa_buffer_left = ((wa_buffer_left + 1) % WATCHASYNC_BUFFERSIZE);
	sendmessage();
      }
    }
  }  
}

/*
  -- Ethersex META --
  header(services/watchasync/watchasync.h)
  init(watchasync_init)
  mainloop(watchasync_mainloop)
*/
