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

// Number of places in ringbuffer, should be a power of 2
#define WATCHASYNC_BUFFERSIZE 64

// first string is the GET part including the path
static const char PROGMEM get_string_head[] =
    "GET " CONF_WATCHASYNC_PATH "?port=";
// next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_WATCHASYNC_INCLUDE_PREFIX
static const char PROGMEM prefix_string[] =
	CONF_WATCHASYNC_PREFIX ;
#endif
// optional uuid
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

// Handle Pinchange Interrupt on PortC
ISR(PCINT2_vect)
{
  uint8_t portcompstate = (PINC ^ wa_portstate); //  compare actual state of PortC with last saved state
  uint8_t pin;	// loop variable for for-loop
  uint8_t tempright;  // temporary pointer for detecting full buffer
  while (portcompstate)  // repeat comparison as long as there are changes to the PortC
  {
    for (pin = 0; pin < 8; pin ++)  // iterate through pins
    {
      if (portcompstate & wa_portstate & (1 << pin)) // bit changed from 1 to 0
      {
        tempright = ((wa_buffer_right + 1) % WATCHASYNC_BUFFERSIZE);  // calculate next position in ringbuffer
	if (tempright != wa_buffer_left)  // if ringbuffer not full
	{
	  wa_buffer_right = tempright;  // select next space in ringbuffer
	  wa_buffer[wa_buffer_right].pin = pin;  // set pin in ringbuffer
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
          wa_buffer[wa_buffer_right].timestamp = clock_get_time();  // add timestamp in ringbuffer
#endif
//	} else {  // ringbuffer is full... discard event
//	  WATCHASYNC_DEBUG ("Buffer full, discarding message!\n");
	}
      }
    }
    wa_portstate ^= portcompstate;  // incorporate changes processed in current state
    portcompstate = (PINC ^ wa_portstate);  // check for new changes on PortC
  }
}

static void watchasync_net_main(void)  // Network-routine called by networkstack 
{
  if (uip_aborted() || uip_timedout()) // Connection aborted or timedout
  {
    // if connectionstate is new, we have to resend the packet, otherwise just ignore the event
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW)
    {
      wa_sendstate = 2; // Ignore aborted, if already closed
      uip_conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_OLD;
      WATCHASYNC_DEBUG ("connection aborted\n");
      return;
    }
  }

  if (uip_closed()) // Closed connection does not expect any respond from us, resend if connnectionstate is new
  {
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW)
    {
      wa_sendstate = 2; // Ignore aborted, if already closed
      uip_conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_OLD;
      WATCHASYNC_DEBUG ("new connection closed\n");
    } else {
      WATCHASYNC_DEBUG ("connection closed\n");
    }
    return;
  }


  if (uip_connected() || uip_rexmit()) { // (re-)transmit packet
    WATCHASYNC_DEBUG ("new connection or rexmit, sending message\n");
    char *p = uip_appdata;  // pointer set to uip_appdata, used to store string
    p += sprintf_P(p, get_string_head);  // Copy Header from programm memory to appdata
#ifdef CONF_WATCHASYNC_INCLUDE_PREFIX
    p += sprintf_P(p, prefix_string);  // Append Prefixstring if configured
#endif
    p += sprintf(p, "%u", wa_buffer[wa_buffer_left].pin);  // append pin changed (0-7)
#ifdef CONF_WATCHASYNC_INCLUDE_UUID
    p += sprintf_P(p, uuid_string);  // append uuid if configured
#endif
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP  
    p += sprintf_P(p, time_string);  // append timestamp attribute
    p += sprintf(p, "%lu", wa_buffer[wa_buffer_left].timestamp); // and timestamp value
#endif
    p += sprintf_P(p, get_string_foot); // appen tail of packet from programmmemory
//    uip_udp_send(p - (char *)uip_appdata);
    uip_udp_send(p - (char *)uip_appdata);
    WATCHASYNC_DEBUG ("send %d bytes\n", p - (char *)uip_appdata);
//    WATCHASYNC_DEBUG ("send %s \n", uip_appdata);
  }

  if (uip_acked()) // Send packet acked, 
  {
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW) // If packet is still new
    {
      wa_sendstate = 0;  // Mark event as sent, go ahead in buffer
      uip_conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_OLD; // mark this packet as old, do noch resend it
      uip_close();  // initiate closing of the connection
      WATCHASYNC_DEBUG ("packet sent, closing\n");
      return;
    } else {
      uip_abort();  // abort connection if old connection received an ack... this should not happen
    }
  }
}


static void watchasync_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)  // Callback for DNS query
{
  WATCHASYNC_DEBUG ("got dns response, connecting\n");
  uip_conn_t *conn = uip_connect(ipaddr, HTONS(80), watchasync_net_main);  // create new connection with ipaddr found
  if(conn)  // if connection succesfully created
  {
    conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_NEW; // Set connection state to new, as data still has to be send
  } else {
    wa_sendstate = 2;  // if no connection initiated, set state to Retry
  }
}

void sendmessage(void) // Send event in ringbuffer indicated by left pointer
{
  wa_sendstate = 1; // set new state in progress

  uip_ipaddr_t *ipaddr; 
  if (!(ipaddr = resolv_lookup(CONF_WATCHASYNC_SERVICE))) { // Try to find IPAddress
    resolv_query(CONF_WATCHASYNC_SERVICE, watchasync_dns_query_cb); // If not found: query DNS
  } else {
    watchasync_dns_query_cb(NULL, ipaddr); // If found use IPAddress
  }
  return;
}

void watchasync_init(void)  // Initilize Poirts and Interrupts
{
  PORTC = (1<<PC7)|(1<<PC6)|(1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1)|(1<<PC0);  // Enable Pull-up on PortC
  DDRC = 0; 			// PortC Input
  wa_portstate = PINC; 		// save current state
  PCMSK2 = (1<<PCINT23)|(1<<PCINT22)|(1<<PCINT21)|(1<<PCINT20)|(1<<PCINT19)|(1<<PCINT18)|(1<<PCINT17)|(1<<PCINT16);  // Enable Pinchange Interrupt on PortC
  PCICR |= 1<<PCIE2;		// Enable Pinchange Interrupt on PortC
//  SREG |= 1<<I;			//Enable Interrupts (will hopefully be done somewhere else)
}

void watchasync_mainloop(void)  // Mainloop routine poll ringsbuffer
{
  if (wa_sendstate != 1) // not busy sending 
  {
    if (wa_sendstate == 2) // Message not sent successfully
    {
      WATCHASYNC_DEBUG ("Error, again please...\n"); 
      sendmessage();   // resend current event
    } else // sendstate == 0 => Idle  // Previous send has been succesfull, send next event if any
    {
      if (wa_buffer_left != wa_buffer_right) // there is somethiing in the buffer
      {
        wa_buffer_left = ((wa_buffer_left + 1) % WATCHASYNC_BUFFERSIZE); // calculate next place in buffer
        WATCHASYNC_DEBUG ("Starting Transmission: L: %u R: %u Pin: %u\n", wa_buffer_left, wa_buffer_right, wa_buffer[wa_buffer_left].pin); 
	sendmessage();  // send the new event
      }
    }
  }  
}

/*
  -- Ethersex META --
  header(services/watchasync/watchasync.h)
  init(watchasync_init)
  mainloop(watchasync_mainloop)
  state_header(services/watchasync/watchasync_state.h)
  state_tcp(`struct watchasync_connection_state_t watchasync;')
*/
