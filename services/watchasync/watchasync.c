
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

#ifdef CONF_WATCHASYNC_TIMESTAMP
#include "services/clock/clock.h"
#endif

// first string is the GET part including the path
static const char PROGMEM watchasync_path[] =
    "GET " CONF_WATCHASYNC_PATH ;
// next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_WATCHASYNC_TIMESTAMP
static const char PROGMEM watchasync_timestamp_path[] =
	CONF_WATCHASYNC_TIMESTAMP_PATH ;
#endif

// and the http footer including the http protocol version and the server name
static const char PROGMEM watchasync_request_end[] =
    " HTTP/1.1\r\n"
    "Host: " CONF_WATCHASYNC_SERVER "\r\n\r\n";
#ifndef CONF_WATCHASYNC_PORT
#define CONF_WATCHASYNC_PORT 80
#endif

#ifdef CONF_WATCHASYNC_PA0
static const char PROGMEM watchasync_ID_PA0[] = CONF_WATCHASYNC_PA0_ID;
#endif
#ifdef CONF_WATCHASYNC_PA1
static const char PROGMEM watchasync_ID_PA1[] = CONF_WATCHASYNC_PA1_ID;
#endif
#ifdef CONF_WATCHASYNC_PA2
static const char PROGMEM watchasync_ID_PA2[] = CONF_WATCHASYNC_PA2_ID;
#endif
#ifdef CONF_WATCHASYNC_PA3
static const char PROGMEM watchasync_ID_PA3[] = CONF_WATCHASYNC_PA3_ID;
#endif
#ifdef CONF_WATCHASYNC_PA4
static const char PROGMEM watchasync_ID_PA4[] = CONF_WATCHASYNC_PA4_ID;
#endif
#ifdef CONF_WATCHASYNC_PA5
static const char PROGMEM watchasync_ID_PA5[] = CONF_WATCHASYNC_PA5_ID;
#endif
#ifdef CONF_WATCHASYNC_PA6
static const char PROGMEM watchasync_ID_PA6[] = CONF_WATCHASYNC_PA6_ID;
#endif
#ifdef CONF_WATCHASYNC_PB7
static const char PROGMEM watchasync_ID_PA7[] = CONF_WATCHASYNC_PA7_ID;
#endif
#ifdef CONF_WATCHASYNC_PB0
static const char PROGMEM watchasync_ID_PB0[] = CONF_WATCHASYNC_PB0_ID;
#endif
#ifdef CONF_WATCHASYNC_PB1
static const char PROGMEM watchasync_ID_PB1[] = CONF_WATCHASYNC_PB1_ID;
#endif
#ifdef CONF_WATCHASYNC_PB2
static const char PROGMEM watchasync_ID_PB2[] = CONF_WATCHASYNC_PB2_ID;
#endif
#ifdef CONF_WATCHASYNC_PB3
static const char PROGMEM watchasync_ID_PB3[] = CONF_WATCHASYNC_PB3_ID;
#endif
#ifdef CONF_WATCHASYNC_PB4
static const char PROGMEM watchasync_ID_PB4[] = CONF_WATCHASYNC_PB4_ID;
#endif
#ifdef CONF_WATCHASYNC_PB5
static const char PROGMEM watchasync_ID_PB5[] = CONF_WATCHASYNC_PB5_ID;
#endif
#ifdef CONF_WATCHASYNC_PB6
static const char PROGMEM watchasync_ID_PB6[] = CONF_WATCHASYNC_PB6_ID;
#endif
#ifdef CONF_WATCHASYNC_PB7
static const char PROGMEM watchasync_ID_PB7[] = CONF_WATCHASYNC_PB7_ID;
#endif
#ifdef CONF_WATCHASYNC_PC0
static const char PROGMEM watchasync_ID_PC0[] = CONF_WATCHASYNC_PC0_ID;
#endif
#ifdef CONF_WATCHASYNC_PC1
static const char PROGMEM watchasync_ID_PC1[] = CONF_WATCHASYNC_PC1_ID;
#endif
#ifdef CONF_WATCHASYNC_PC2
static const char PROGMEM watchasync_ID_PC2[] = CONF_WATCHASYNC_PC2_ID;
#endif
#ifdef CONF_WATCHASYNC_PC3
static const char PROGMEM watchasync_ID_PC3[] = CONF_WATCHASYNC_PC3_ID;
#endif
#ifdef CONF_WATCHASYNC_PC4
static const char PROGMEM watchasync_ID_PC4[] = CONF_WATCHASYNC_PC4_ID;
#endif
#ifdef CONF_WATCHASYNC_PC5
static const char PROGMEM watchasync_ID_PC5[] = CONF_WATCHASYNC_PC5_ID;
#endif
#ifdef CONF_WATCHASYNC_PC6
static const char PROGMEM watchasync_ID_PC6[] = CONF_WATCHASYNC_PC6_ID;
#endif
#ifdef CONF_WATCHASYNC_PC7
static const char PROGMEM watchasync_ID_PC7[] = CONF_WATCHASYNC_PC7_ID;
#endif
#ifdef CONF_WATCHASYNC_PD0
static const char PROGMEM watchasync_ID_PD0[] = CONF_WATCHASYNC_PD0_ID;
#endif
#ifdef CONF_WATCHASYNC_PD1
static const char PROGMEM watchasync_ID_PD1[] = CONF_WATCHASYNC_PD1_ID;
#endif
#ifdef CONF_WATCHASYNC_PD2
static const char PROGMEM watchasync_ID_PD2[] = CONF_WATCHASYNC_PD2_ID;
#endif
#ifdef CONF_WATCHASYNC_PD3
static const char PROGMEM watchasync_ID_PD3[] = CONF_WATCHASYNC_PD3_ID;
#endif
#ifdef CONF_WATCHASYNC_PD4
static const char PROGMEM watchasync_ID_PD4[] = CONF_WATCHASYNC_PD4_ID;
#endif
#ifdef CONF_WATCHASYNC_PD5
static const char PROGMEM watchasync_ID_PD5[] = CONF_WATCHASYNC_PD5_ID;
#endif
#ifdef CONF_WATCHASYNC_PD6
static const char PROGMEM watchasync_ID_PD6[] = CONF_WATCHASYNC_PD6_ID;
#endif
#ifdef CONF_WATCHASYNC_PD7
static const char PROGMEM watchasync_ID_PD7[] = CONF_WATCHASYNC_PD7_ID;
#endif

PGM_P watchasync_ID[] PROGMEM =
{
#ifdef CONF_WATCHASYNC_PA0
  watchasync_ID_PA0
#if WATCHASYNC_PA0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA1
  watchasync_ID_PA1
#if WATCHASYNC_PA1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA2
  watchasync_ID_PA2
#if WATCHASYNC_PA2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA3
  watchasync_ID_PA3
#if WATCHASYNC_PA3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA4
  watchasync_ID_PA4
#if WATCHASYNC_PA4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA5
  watchasync_ID_PA5
#if WATCHASYNC_PA5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA6
  watchasync_ID_PA6
#if WATCHASYNC_PA6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PA7
  watchasync_ID_PA7
#if WATCHASYNC_PA7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB0
  watchasync_ID_PB0
#if WATCHASYNC_PB0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB1
  watchasync_ID_PB1
#if WATCHASYNC_PB1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB2
  watchasync_ID_PB2
#if WATCHASYNC_PB2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB3
  watchasync_ID_PB3
#if WATCHASYNC_PB3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB4
  watchasync_ID_PB4
#if WATCHASYNC_PB4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB5
  watchasync_ID_PB5
#if WATCHASYNC_PB5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB6
  watchasync_ID_PB6
#if WATCHASYNC_PB6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PB7
  watchasync_ID_PB7
#if WATCHASYNC_PB7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC0
  watchasync_ID_PC0
#if WATCHASYNC_PC0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC1
  watchasync_ID_PC1
#if WATCHASYNC_PC1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC2
  watchasync_ID_PC2
#if WATCHASYNC_PC2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC3
  watchasync_ID_PC3
#if WATCHASYNC_PC3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC4
  watchasync_ID_PC4
#if WATCHASYNC_PC4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC5
  watchasync_ID_PC5
#if WATCHASYNC_PC5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC6
  watchasync_ID_PC6
#if WATCHASYNC_PC6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PC7
  watchasync_ID_PC7
#if WATCHASYNC_PC7_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD0
  watchasync_ID_PD0
#if WATCHASYNC_PD0_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD1
  watchasync_ID_PD1
#if WATCHASYNC_PD1_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD2
  watchasync_ID_PD2
#if WATCHASYNC_PD2_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD3
  watchasync_ID_PD3
#if WATCHASYNC_PD3_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD4
  watchasync_ID_PD4
#if WATCHASYNC_PD4_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD5
  watchasync_ID_PD5
#if WATCHASYNC_PD5_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD6
  watchasync_ID_PD6
#if WATCHASYNC_PD6_INDEX < (WATCHASYNC_PINCOUNT-1)
  ,
#endif
#endif
#ifdef CONF_WATCHASYNC_PD7
  watchasync_ID_PD7
#endif
};

static struct WatchAsyncBuffer wa_buffer[CONF_WATCHASYNC_BUFFERSIZE]; // Ringbuffer for Messages
static uint8_t wa_buffer_left = 0; 	// last position sent
static uint8_t wa_buffer_right = 0; 	// last position set

//static uint8_t wa_portstate = 0; 		// Last portstate saved
static uint8_t wa_sendstate = 0; 		// 0: Idle, 1: Message being sent, 2: Sending message failed

void addToRingbuffer(int pin)
{
    uint8_t tempright;  // temporary pointer for detecting full buffer
    tempright = ((wa_buffer_right + 1) % CONF_WATCHASYNC_BUFFERSIZE);  // calculate next position in ringbuffer
    if (tempright != wa_buffer_left)  // if ringbuffer not full
    {
        wa_buffer_right = tempright;  // select next space in ringbuffer
        wa_buffer[wa_buffer_right].pin = pin;  // set pin in ringbuffer
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
#if CONF_WATCHASYNC_RESOLUTION > 1
//        wa_buffer[wa_buffer_right].timestamp = ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) * CONF_WATCHASYNC_RESOLUTION;  // add timestamp in ringbuffer
        wa_buffer[wa_buffer_right].timestamp = ( clock_get_time() & (uint32_t) (-1 * CONF_WATCHASYNC_RESOLUTION )  // add timestamp in ringbuffer
#else
        wa_buffer[wa_buffer_right].timestamp = clock_get_time();  // add timestamp in ringbuffer
#endif
#endif
    }
}

#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING

static uint8_t idx = 0;
#ifdef CONF_WATCHASYNC_PA
static uint8_t stateA = 0;
static uint8_t samplesA[3] = {0,0,0};
#endif
#ifdef CONF_WATCHASYNC_PB
static uint8_t stateB = 0;
static uint8_t samplesB[3] = {0,0,0};
#endif
#ifdef CONF_WATCHASYNC_PC
static uint8_t stateC = 0;
static uint8_t samplesC[3] = {0,0,0};
#endif
#ifdef CONF_WATCHASYNC_PD
static uint8_t stateD = 0;
static uint8_t samplesD[3] = {0,0,0};
#endif

// the main purpose of the function is detect rasing edges and put them
// in the ring buffer. In order to debounce for signals which are not perfect
// we require 3 consecutive samples with the same level
void watchasync_periodic(void)
{
    uint8_t StateDiff;

    idx++;
    if (idx>2){idx = 0;}

    // put element into ringbuffer
#ifdef CONF_WATCHASYNC_PA
    samplesA[idx] = PINA;
    // Detect rising Edges having proved stable:
    StateDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesA[0] ^ samplesA[1]) | (samplesA[0] ^ samplesA[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesA[0] ^ stateA)
    // Bits that are set (filter falling edges)
    & samplesA[0]
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PA_MASK;
    if (StateDiff) {
#ifdef CONF_WATCHASYNC_PA0
      if (StateDiff & 1)
        addToRingbuffer(WATCHASYNC_PA0_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA1
      if (StateDiff & 2)
        addToRingbuffer(WATCHASYNC_PA1_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA2
      if (StateDiff & 4)
        addToRingbuffer(WATCHASYNC_PA2_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA3
      if (StateDiff & 8)
        addToRingbuffer(WATCHASYNC_PA3_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA4
      if (StateDiff & 16)
        addToRingbuffer(WATCHASYNC_PA4_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA5
      if (StateDiff & 32)
        addToRingbuffer(WATCHASYNC_PA5_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA6
      if (StateDiff & 64)
        addToRingbuffer(WATCHASYNC_PA6_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PA7
      if (StateDiff & 128)
        addToRingbuffer(WATCHASYNC_PA7_INDEX);
#endif      
      stateA = samplesA[0];
    }
#endif

#ifdef CONF_WATCHASYNC_PB
    samplesB[idx] = PINB;
    // Detect rising Edges having proved stable:
    StateDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesB[0] ^ samplesB[1]) | (samplesB[0] ^ samplesB[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesB[0] ^ stateB)
    // Bits that are set (filter falling edges)
    & samplesB[0]
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PB_MASK;
    if (StateDiff) {
#ifdef CONF_WATCHASYNC_PB0
      if (StateDiff & 1)
        addToRingbuffer(WATCHASYNC_PB0_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB1
      if (StateDiff & 2)
        addToRingbuffer(WATCHASYNC_PB1_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB2
      if (StateDiff & 4)
        addToRingbuffer(WATCHASYNC_PB2_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB3
      if (StateDiff & 8)
        addToRingbuffer(WATCHASYNC_PB3_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB4
      if (StateDiff & 16)
        addToRingbuffer(WATCHASYNC_PB4_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB5
      if (StateDiff & 32)
        addToRingbuffer(WATCHASYNC_PB5_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB6
      if (StateDiff & 64)
        addToRingbuffer(WATCHASYNC_PB6_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PB7
      if (StateDiff & 128)
        addToRingbuffer(WATCHASYNC_PB7_INDEX);
#endif      
      stateB = samplesB[0];
    }
#endif

#ifdef CONF_WATCHASYNC_PC
    samplesC[idx] = PINC;
    // Detect rising Edges having proved stable:
    StateDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesC[0] ^ samplesC[1]) | (samplesC[0] ^ samplesC[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesC[0] ^ stateC)
    // Bits that are set (filter falling edges)
    & samplesC[0]
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PC_MASK;
    if (StateDiff) {
#ifdef CONF_WATCHASYNC_PC0
      if (StateDiff & 1)
        addToRingbuffer(WATCHASYNC_PC0_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC1
      if (StateDiff & 2)
        addToRingbuffer(WATCHASYNC_PC1_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC2
      if (StateDiff & 4)
        addToRingbuffer(WATCHASYNC_PC2_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC3
      if (StateDiff & 8)
        addToRingbuffer(WATCHASYNC_PC3_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC4
      if (StateDiff & 16)
        addToRingbuffer(WATCHASYNC_PC4_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC5
      if (StateDiff & 32)
        addToRingbuffer(WATCHASYNC_PC5_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC6
      if (StateDiff & 64)
        addToRingbuffer(WATCHASYNC_PC6_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PC7
      if (StateDiff & 128)
        addToRingbuffer(WATCHASYNC_PC7_INDEX);
#endif      
      stateC = samplesC[0];
    }
#endif

#ifdef CONF_WATCHASYNC_PD
    samplesD[idx] = PIND;
    // Detect rising Edges having proved stable:
    StateDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesD[0] ^ samplesD[1]) | (samplesD[0] ^ samplesD[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesD[0] ^ stateD)
    // Bits that are set (filter falling edges)
    & samplesD[0]
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PD_MASK;
    if (StateDiff) {
#ifdef CONF_WATCHASYNC_PD0
      if (StateDiff & 1)
        addToRingbuffer(WATCHASYNC_PD0_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD1
      if (StateDiff & 2)
        addToRingbuffer(WATCHASYNC_PD1_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD2
      if (StateDiff & 4)
        addToRingbuffer(WATCHASYNC_PD2_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD3
      if (StateDiff & 8)
        addToRingbuffer(WATCHASYNC_PD3_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD4
      if (StateDiff & 16)
        addToRingbuffer(WATCHASYNC_PD4_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD5
      if (StateDiff & 32)
        addToRingbuffer(WATCHASYNC_PD5_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD6
      if (StateDiff & 64)
        addToRingbuffer(WATCHASYNC_PD6_INDEX);
#endif      
#ifdef CONF_WATCHASYNC_PD7
      if (StateDiff & 128)
        addToRingbuffer(WATCHASYNC_PD7_INDEX);
#endif      
      stateD = samplesD[0];
    }
#endif
}

#else
// Handle Pinchange Interrupt on PortC
ISR(PCINT2_vect)
{
  uint8_t portcompstate = (PINC ^ wa_portstate); //  compare actual state of PortC with last saved state
  uint8_t pin;	// loop variable for for-loop
  while (portcompstate)  // repeat comparison as long as there are changes to the PortC
  {
    for (pin = 0; pin < 8; pin ++)  // iterate through pins
    {
      if (portcompstate & wa_portstate & (1 << pin)) // bit changed from 1 to 0
      {
        addToRingbuffer(pin);
      }
    }
    wa_portstate ^= portcompstate;  // incorporate changes processed in current state
    portcompstate = (PINC ^ wa_portstate);  // check for new changes on PortC
  }
}
#endif /* ! CONF_WATCHASYNC_EDGDETECTVIAPOLLING */

static void watchasync_net_main(void)  // Network-routine called by networkstack 
{
  if (uip_aborted() || uip_timedout() || uip_closed() ) // Connection aborted or timedout
  {
    // if connectionstate is new, we have to resend the packet, otherwise just ignore the event
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW)
    {
      wa_sendstate = 2; // Ignore aborted, if already closed
      uip_conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_OLD;
      WATCHASYNC_DEBUG ("connection aborted\n");
      return;
    } else if (uip_closed()) {
      WATCHASYNC_DEBUG ("connection closed\n");
    }
  }


  if (uip_connected() || uip_rexmit()) { // (re-)transmit packet
    WATCHASYNC_DEBUG ("new connection or rexmit, sending message\n");
    char *p = uip_appdata;  // pointer set to uip_appdata, used to store string
    p += sprintf_P(p, watchasync_path);  // copy path from programm memory to appdata
    p += sprintf_P(p, (PGM_P)pgm_read_word(&(watchasync_ID[wa_buffer[wa_buffer_left].pin])));  // append uuid if configured
#ifdef CONF_WATCHASYNC_TIMESTAMP  
    p += sprintf_P(p, watchasync_timestamp_path);  // append timestamp attribute
    p += sprintf(p, "%lu", wa_buffer[wa_buffer_left].timestamp); // and timestamp value
#endif
    p += sprintf_P(p, watchasync_request_end); // appen tail of packet from programmmemory
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
  uip_conn_t *conn = uip_connect(ipaddr, HTONS(CONF_WATCHASYNC_PORT), watchasync_net_main);  // create new connection with ipaddr found
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

#ifdef DNS_SUPPORT
  uip_ipaddr_t *ipaddr; 
  if (!(ipaddr = resolv_lookup(CONF_WATCHASYNC_SERVER))) { // Try to find IPAddress
    resolv_query(CONF_WATCHASYNC_SERVER, watchasync_dns_query_cb); // If not found: query DNS
  } else {
    watchasync_dns_query_cb(NULL, ipaddr); // If found use IPAddress
  }
#else /* ! DNS_SUPPORT */
  uip_ipaddr_t ip;
  set_WATCHASYNC_SERVER_IP(&ip);

  watchasync_dns_query_cb(NULL, &ip);
#endif

  return;
}

void watchasync_init(void)  // Initilize Poirts and Interrupts
{
#ifdef CONF_WATCHASYNC_PA
  PORTA = WATCHASYNC_PA_MASK;  // Enable Pull-up on PortA
  DDRA = 255 - WATCHASYNC_PA_MASK;  // PortA Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesA[0] = samplesA[1] = samplesA[2] = PINA;  // save current state
#else
  wa_portstateA = PINA;  // save current state
  PCMSK0 = WATCHASYNC_PA_MASK;  // Enable Pinchange Interrupt on PortA
  PCICR |= 1<<PCIE0;  // Enable Pinchange Interrupt on PortA
#endif
#endif

#ifdef CONF_WATCHASYNC_PB
  PORTB = WATCHASYNC_PB_MASK;  // Enable Pull-up on PortB
  DDRB = 255 - WATCHASYNC_PB_MASK;  // PortB Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesB[0] = samplesB[1] = samplesB[2] = PINB;  // save current state
#else
  wa_portstateB = PINB;  // save current state
  PCMSK1 = WATCHASYNC_PB_MASK;  // Enable Pinchange Interrupt on PortA
  PCICR |= 1<<PCIE1;  // Enable Pinchange Interrupt on PortA
#endif
#endif

#ifdef CONF_WATCHASYNC_PC
  PORTC = WATCHASYNC_PC_MASK;  // Enable Pull-up on PortC
  DDRC = 255 - WATCHASYNC_PC_MASK;  // PortC Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesC[0] = samplesC[1] = samplesC[2] = PINC;  // save current state
#else
  wa_portstateC = PINC;  // save current state
  PCMSK2 = WATCHASYNC_PC_MASK;  // Enable Pinchange Interrupt on PortC
  PCICR |= 1<<PCIE2;  // Enable Pinchange Interrupt on PortC
#endif
#endif

#ifdef CONF_WATCHASYNC_PD
  PORTD = WATCHASYNC_PD_MASK;  // Enable Pull-up on PortD
  DDRD = 255 - WATCHASYNC_PD_MASK;  // PortD Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesD[0] = samplesD[1] = samplesD[2] = PIND;  // save current state
#else
  wa_portstateD = PIND;  // save current state
  PCMSK3 = WATCHASYNC_PD_MASK;  // Enable Pinchange Interrupt on PortD
  PCICR |= 1<<PCIE3;  // Enable Pinchange Interrupt on PortD
#endif
#endif

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
        wa_buffer_left = ((wa_buffer_left + 1) % CONF_WATCHASYNC_BUFFERSIZE); // calculate next place in buffer
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
  timer(1, watchasync_periodic())
*/
