
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
#else // def CONF_WATCHASYNC_TIMESTAMP
#ifdef CONF_WATCHASYNC_SUMMARIZE
#include "services/clock/clock.h"
#endif // def CONF_WATCHASYNC_SUMMARIZE
#endif // def CONF_WATCHASYNC_TIMESTAMP

#include "services/watchasync/watchasync_strings.c"

static struct WatchAsyncBuffer wa_buffer[CONF_WATCHASYNC_BUFFERSIZE]; // Ringbuffer for Messages
#ifdef CONF_WATCHASYNC_SUMMARIZE
static uint8_t wa_buf;  // bufferposition to send
static uint8_t wa_bufpin; // pin of bufferposition to send
#else // def CONF_WATCHASYNC_SUMMARIZE
static uint8_t wa_buffer_left = 0; 	// last position sent
static uint8_t wa_buffer_right = 0; 	// last position set
#endif // def CONF_WATCHASYNC_SUMMARIZE

static uint8_t wa_sendstate = 0; 		// 0: Idle, 1: Message being sent, 2: Sending message failed

void addToRingbuffer(int pin)
{
    uint8_t tempright;  // temporary pointer for detecting full buffer
#ifdef CONF_WATCHASYNC_SUMMARIZE
#if CONF_WATCHASYNC_RESOLUTION > 1
#ifdef CONF_WATCHASYNC_SENDEND
    tempright = ( ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) + 1 ) % CONF_WATCHASYNC_BUFFERSIZE;
#else // def CONF_WATCHASYNC_SENDEND
    tempright = ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) % CONF_WATCHASYNC_BUFFERSIZE;
#endif // def CONF_WATCHASYNC_SENDEND
#else // CONF_WATCHASYNC_RESOLUTION > 1
#ifdef CONF_WATCHASYNC_SENDEND
    tempright = ( clock_get_time() + 1) % CONF_WATCHASYNC_BUFFERSIZE;
#else // def CONF_WATCHASYNC_SENDEND
    tempright = clock_get_time() % CONF_WATCHASYNC_BUFFERSIZE;
#endif // def CONF_WATCHASYNC_SENDEND
#endif // CONF_WATCHASYNC_RESOLUTION > 1
    if ((~wa_buffer[tempright].pin[pin]) != 0) // check for overflow
    {
        wa_buffer[tempright].pin[pin] ++;
    }
#else // def CONF_WATCHASYNC_SUMMARIZE
    tempright = ((wa_buffer_right + 1) % CONF_WATCHASYNC_BUFFERSIZE);  // calculate next position in ringbuffer
    if (tempright != wa_buffer_left)  // if ringbuffer not full
    {
        wa_buffer_right = tempright;  // select next space in ringbuffer
        wa_buffer[wa_buffer_right].pin = pin;  // set pin in ringbuffer
#ifdef CONF_WATCHASYNC_TIMESTAMP
#if CONF_WATCHASYNC_RESOLUTION > 1
//        wa_buffer[wa_buffer_right].timestamp = ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) * CONF_WATCHASYNC_RESOLUTION;  // add timestamp in ringbuffer
#ifdef CONF_WATCHASYNC_SENDEND
	wa_buffer[wa_buffer_right].timestamp = clock_get_time() & ( (uint32_t) (-1 * CONF_WATCHASYNC_RESOLUTION )) + CONF_WATCHASYNC_RESOLUTION; // add timestamp in ringbuffer
#else // def CONF_WATCHASYNC_SENDEND
	wa_buffer[wa_buffer_right].timestamp = clock_get_time() & ( (uint32_t) (-1 * CONF_WATCHASYNC_RESOLUTION )); // add timestamp in ringbuffer
#endif // def CONF_WATCHASYNC_SENDEND
#else // CONF_WATCHASYNC_RESOLUTION > 1
        wa_buffer[wa_buffer_right].timestamp = clock_get_time();  // add timestamp in ringbuffer
#endif // CONF_WATCHASYNC_RESOLUTION > 1
#endif // def CONF_WATCHASYNC_TIMESTAMP
    }
#endif // def CONF_WATCHASYNC_SUMMARIZE
}

#ifdef CONF_WATCHASYNC_PA
static uint8_t stateA = 255;
#endif // def CONF_WATCHASYNC_PA
#ifdef CONF_WATCHASYNC_PB
static uint8_t stateB = 255;
#endif // def CONF_WATCHASYNC_PB
#ifdef CONF_WATCHASYNC_PC
static uint8_t stateC = 255;
#endif // def CONF_WATCHASYNC_PC
#ifdef CONF_WATCHASYNC_PD
static uint8_t stateD = 255;
#endif // def CONF_WATCHASYNC_PD


// polling mechanism goes first
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING

static uint8_t idx = 0;
#ifdef CONF_WATCHASYNC_PA
static uint8_t samplesA[3] = {255,0,0};
#endif // def CONF_WATCHASYNC_PA
#ifdef CONF_WATCHASYNC_PB
static uint8_t samplesB[3] = {255,0,0};
#endif // def CONF_WATCHASYNC_PB
#ifdef CONF_WATCHASYNC_PC
static uint8_t samplesC[3] = {255,0,0};
#endif // def CONF_WATCHASYNC_PC
#ifdef CONF_WATCHASYNC_PD
static uint8_t samplesD[3] = {255,0,0};
#endif // def CONF_WATCHASYNC_PD

// the main purpose of the function is detect rasing edges and put them
// in the ring buffer. In order to debounce for signals which are not perfect
// we require 3 consecutive samples with the same level
void watchasync_periodic(void)
{
    uint8_t StateDiff;
    uint8_t TempDiff;

    idx++;
    if (idx>2){idx = 0;}

    // put element into ringbuffer
#ifdef CONF_WATCHASYNC_PA
    samplesA[idx] = PINA;
    // Detect changes having proved stable:
    TempDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesA[0] ^ samplesA[1]) | (samplesA[0] ^ samplesA[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesA[0] ^ stateA)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PA_MASK;
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
    // Bits that are set (filter falling edges)
    & samplesA[0];

    if (StateDiff) {
#ifdef CONF_WATCHASYNC_PA0
      if (StateDiff & 1)
        addToRingbuffer(WATCHASYNC_PA0_INDEX);
#endif // def CONF_WATCHASYNC_PA0
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
    }
    stateA ^= TempDiff;
#endif

#ifdef CONF_WATCHASYNC_PB
    samplesB[idx] = PINB;
    // Detect changes having proved stable:
    TempDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesB[0] ^ samplesB[1]) | (samplesB[0] ^ samplesB[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesB[0] ^ stateB)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PB_MASK;
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
    // Bits that are set (filter falling edges)
    & samplesB[0];

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
    }
    stateB ^= TempDiff;
#endif

#ifdef CONF_WATCHASYNC_PC
    samplesC[idx] = PINC;
    // Detect changes having proved stable:
    TempDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesC[0] ^ samplesC[1]) | (samplesC[0] ^ samplesC[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesC[0] ^ stateC)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PC_MASK;
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
    // Bits that are set (filter falling edges)
    & samplesC[0];

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
    }
    stateC ^= TempDiff;
#endif

#ifdef CONF_WATCHASYNC_PD
    samplesD[idx] = PIND;
    // Detect changes having proved stable:
    TempDiff = 
    // Zerobits mark unstable Bits:
    ~( (samplesD[0] ^ samplesD[1]) | (samplesD[0] ^ samplesD[2]) )
    // Bits that have changed (filter unchanged Bits)
    & (samplesD[0] ^ stateD)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PD_MASK;
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
    // Bits that are set (filter falling edges)
    & samplesD[0];

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
    }
    stateD ^= TempDiff;
#endif
}




/////////////////////////////////////////////////
// Interrupt driven routines
/////////////////////////////////////////////////

#else  /* ! CONF_WATCHASYNC_EDGDETECTVIAPOLLING */

#ifdef CONF_WATCHASYNC_PA
// Handle Pinchange Interrupt on PortA
ISR(PCINT0_vect)
{
  uint8_t StateDiff;
  uint8_t TempDiff;
  uint8_t PinState = PINA;
  // Detect changes having proved stable:
  TempDiff = 
    // Bits that have changed (filter unchanged Bits)
    (PinState ^ stateA)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PA_MASK;

  while (TempDiff) {
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
      // Bits that are set (filter falling edges)
      & PinState;
    if (StateDiff)
    {
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
    }
    stateA ^= TempDiff;

    PinState = PINA;
    // Detect changes having proved stable:
    TempDiff = 
      // Bits that have changed (filter unchanged Bits)
      (PinState ^ stateA)
      // Bits in our Mask (Filter unwatched bits)
      & WATCHASYNC_PA_MASK;
  }
}
#endif



#ifdef CONF_WATCHASYNC_PB
// Handle Pinchange Interrupt on PortB
ISR(PCINT1_vect)
{
  uint8_t StateDiff;
  uint8_t TempDiff;
  uint8_t PinState = PINB;
  // Detect changes having proved stable:
  TempDiff = 
    // Bits that have changed (filter unchanged Bits)
    (PinState ^ stateB)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PB_MASK;

  while (TempDiff) {
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
      // Bits that are set (filter falling edges)
      & PinState;
    if (StateDiff)
    {
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
    }
    stateB ^= TempDiff;

    PinState = PINB;
    // Detect changes having proved stable:
    TempDiff = 
      // Bits that have changed (filter unchanged Bits)
      (PinState ^ stateB)
      // Bits in our Mask (Filter unwatched bits)
      & WATCHASYNC_PB_MASK;
  }
}
#endif




#ifdef CONF_WATCHASYNC_PC
// Handle Pinchange Interrupt on PortC
ISR(PCINT2_vect)
{
  uint8_t StateDiff;
  uint8_t TempDiff;
  uint8_t PinState = PINC;
  // Detect changes having proved stable:
  TempDiff = 
    // Bits that have changed (filter unchanged Bits)
    (PinState ^ stateC)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PC_MASK;

  while (TempDiff) {
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
      // Bits that are set (filter falling edges)
      & PinState;
    if (StateDiff)
    {
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
    }
    stateC ^= TempDiff;

    PinState = PINC;
    // Detect changes having proved stable:
    TempDiff = 
      // Bits that have changed (filter unchanged Bits)
      (PinState ^ stateC)
      // Bits in our Mask (Filter unwatched bits)
      & WATCHASYNC_PC_MASK;
  }
}
#endif



#ifdef CONF_WATCHASYNC_PD
// Handle Pinchange Interrupt on PortD
ISR(PCINT3_vect)
{
  uint8_t StateDiff;
  uint8_t TempDiff;
  uint8_t PinState = PIND;
  // Detect changes having proved stable:
  TempDiff = 
    // Bits that have changed (filter unchanged Bits)
    (PinState ^ stateD)
    // Bits in our Mask (Filter unwatched bits)
    & WATCHASYNC_PD_MASK;

  while (TempDiff) {
    // Detect rising edges having proved stable:
    StateDiff = TempDiff
      // Bits that are set (filter falling edges)
      & PinState;
    if (StateDiff)
    {
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
    }
    stateD ^= TempDiff;

    PinState = PIND;
    // Detect changes having proved stable:
    TempDiff = 
      // Bits that have changed (filter unchanged Bits)
      (PinState ^ stateD)
      // Bits in our Mask (Filter unwatched bits)
      & WATCHASYNC_PD_MASK;
  }
}
#endif

#endif /* ! CONF_WATCHASYNC_EDGDETECTVIAPOLLING */






////////////////////////////////////////////////////////////
/// Send Data
////////////////////////////////////////////////////////////

static void watchasync_net_main(void)  // Network-routine called by networkstack 
{
  if (uip_aborted() || uip_timedout() || uip_closed() ) // Connection aborted or timedout
  {
    // if connectionstate is new, we have to resend the packet, otherwise just ignore the event
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW)
    {
#ifdef CONF_WATCHASYNC_SUMMARIZE
#if CONF_WATCHASYNC_RESOLUTION > 1
      uint8_t buf = ( uip_conn->appstate.watchasync.timestamp / CONF_WATCHASYNC_RESOLUTION ) % CONF_WATCHASYNC_BUFFERSIZE;
#else // CONF_WATCHASYNC_RESOLUTION > 1
      uint8_t buf = uip_conn->appstate.watchasync.timestamp % CONF_WATCHASYNC_BUFFERSIZE;
#endif // CONF_WATCHASYNC_RESOLUTION > 1
      wa_buffer[buf].pin[uip_conn->appstate.watchasync.pin] += uip_conn->appstate.watchasync.count;
#else // def CONF_WATCHASYNC_SUMMARIZE
      wa_sendstate = 2; // Ignore aborted, if already closed
#endif // def CONF_WATCHASYNC_SUMMARIZE
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
#ifdef CONF_WATCHASYNC_SUMMARIZE
    p += sprintf_P(p, (PGM_P) pgm_read_word(&(watchasync_ID[uip_conn->appstate.watchasync.pin])));  // append uuid if configured
#else // def CONF_WATCHASYNC_SUMMARIZE
    p += sprintf_P(p, (PGM_P) pgm_read_word(&(watchasync_ID[wa_buffer[wa_buffer_left].pin])));  // append uuid if configured
#endif // def CONF_WATCHASYNC_SUMMARIZE
#ifdef CONF_WATCHASYNC_TIMESTAMP  
    p += sprintf_P(p, watchasync_timestamp_path);  // append timestamp attribute
#ifdef CONF_WATCHASYNC_SUMMARIZE
    p += sprintf(p, "%lu", uip_conn->appstate.watchasync.timestamp); // and timestamp value
#else // def CONF_WATCHASYNC_SUMMARIZE
    p += sprintf(p, "%lu", wa_buffer[wa_buffer_left].timestamp); // and timestamp value
#endif // def CONF_WATCHASYNC_SUMMARIZE
#endif // def CONF_WATCHASYNC_TIMESTAMP
#ifdef CONF_WATCHASYNC_SUMMARIZE
    p += sprintf_P(p, watchasync_summarize_path);  // append timestamp attribute
    p += sprintf(p,  WATCHASYNC_COUNTER_FORMAT , uip_conn->appstate.watchasync.count); // and timestamp value
#endif // def CONF_WATCHASYNC_SUMMARIZE
    p += sprintf_P(p, watchasync_request_end); // append tail of packet from programmmemory
//    uip_udp_send(p - (char *)uip_appdata);
    uip_udp_send(p - (char *)uip_appdata);

    WATCHASYNC_DEBUG ("send %d bytes\n", p - (char *)uip_appdata);
  }

  if (uip_acked()) // Send packet acked, 
  {
    if (uip_conn->appstate.watchasync.state == WATCHASYNC_CONNSTATE_NEW) // If packet is still new
    {
#ifndef CONF_WATCHASYNC_SUMMARIZE
      wa_sendstate = 0;  // Mark event as sent, go ahead in buffer
#endif      
      uip_conn->appstate.watchasync.state = WATCHASYNC_CONNSTATE_OLD; // mark this packet as old, do not resend it
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
#ifdef CONF_WATCHASYNC_SUMMARIZE
#if CONF_WATCHASYNC_RESOLUTION > 1
//    conn->appstate.watchasync.timestamp = (clock_get_time() & (uint32_t) (-1 * CONF_WATCHASYNC_BUFFERSIZE * CONF_WATCHASYNC_RESOLUTION)) + wa_buf * CONF_WATCHASYNC_RESOLUTION;
    conn->appstate.watchasync.timestamp = ((clock_get_time() / (uint32_t) ((uint32_t) CONF_WATCHASYNC_RESOLUTION * (uint32_t) CONF_WATCHASYNC_BUFFERSIZE)) * (uint32_t) ((uint32_t) CONF_WATCHASYNC_RESOLUTION * (uint32_t) CONF_WATCHASYNC_BUFFERSIZE)) + (uint32_t) (wa_buf * (uint32_t) CONF_WATCHASYNC_RESOLUTION);
    if (conn->appstate.watchasync.timestamp > clock_get_time() ) conn->appstate.watchasync.timestamp -= (uint32_t) ((uint32_t) CONF_WATCHASYNC_BUFFERSIZE * (uint32_t) CONF_WATCHASYNC_RESOLUTION);
#else // CONF_WATCHASYNC_RESOLUTION > 1
//    conn->appstate.watchasync.timestamp = (clock_get_time() & (uint32_t) (-1 * CONF_WATCHASYNC_BUFFERSIZE)) + wa_buf;
    conn->appstate.watchasync.timestamp = ((clock_get_time() / CONF_WATCHASYNC_BUFFERSIZE) * CONF_WATCHASYNC_BUFFERSIZE) + wa_buf;
    if (conn->appstate.watchasync.timestamp > clock_get_time() ) conn->appstate.watchasync.timestamp -= CONF_WATCHASYNC_BUFFERSIZE;
#endif // CONF_WATCHASYNC_RESOLUTION > 1
    conn->appstate.watchasync.pin = wa_bufpin;
    conn->appstate.watchasync.count = wa_buffer[wa_buf].pin[wa_bufpin];
    wa_buffer[wa_buf].pin[wa_bufpin] -= conn->appstate.watchasync.count;
    wa_sendstate = 0;
#endif // def CONF_WATCHASYNC_SUMMARIZE
  } else {
#ifdef CONF_WATCHASYNC_SUMMARIZE
    wa_sendstate = 0;  // if no connection initiated, set state to Retry
#else
    wa_sendstate = 2;  // if no connection initiated, set state to Retry
#endif // def CONF_WATCHASYNC_SUMMARIZE
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
#else // def DNS_SUPPORT
  uip_ipaddr_t ip;
  set_WATCHASYNC_SERVER_IP(&ip);

  watchasync_dns_query_cb(NULL, &ip);
#endif // def DNS_SUPPORT

  return;
}

void watchasync_init(void)  // Initialize Ports and Interrupts
{
#ifdef CONF_WATCHASYNC_PA
  PORTA = WATCHASYNC_PA_MASK;  // Enable Pull-up on PortA
  DDRA = 255 - WATCHASYNC_PA_MASK;  // PortA Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesA[0] = samplesA[1] = samplesA[2] = PINA;  // save current state
#else // def CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  PCMSK0 = WATCHASYNC_PA_MASK;  // Enable Pinchange Interrupt on PortA
  PCICR |= 1<<PCIE0;  // Enable Pinchange Interrupt on PortA
#endif // def CONF_WATCHASYNC_EDGDETECTVIAPOLLING
#endif // def CONF_WATCHASYNC_PA

#ifdef CONF_WATCHASYNC_PB
  PORTB = WATCHASYNC_PB_MASK;  // Enable Pull-up on PortB
  DDRB = 255 - WATCHASYNC_PB_MASK;  // PortB Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesB[0] = samplesB[1] = samplesB[2] = PINB;  // save current state
#else
  PCMSK1 = WATCHASYNC_PB_MASK;  // Enable Pinchange Interrupt on PortB
  PCICR |= 1<<PCIE1;  // Enable Pinchange Interrupt on PortB
#endif
#endif

#ifdef CONF_WATCHASYNC_PC
  PORTC = WATCHASYNC_PC_MASK;  // Enable Pull-up on PortC
  DDRC = 255 - WATCHASYNC_PC_MASK;  // PortC Input
#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
  samplesC[0] = samplesC[1] = samplesC[2] = PINC;  // save current state
#else
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
  PCMSK3 = WATCHASYNC_PD_MASK;  // Enable Pinchange Interrupt on PortD
  PCICR |= 1<<PCIE3;  // Enable Pinchange Interrupt on PortD
#endif
#endif

}

void watchasync_mainloop(void)  // Mainloop routine poll ringsbuffer
{
  if (wa_sendstate != 1) // not busy sending 
  {
#ifdef CONF_WATCHASYNC_SUMMARIZE
#if CONF_WATCHASYNC_RESOLUTION > 1
#ifdef CONF_WATCHASYNC_SENDEND
    uint8_t temp = ( ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) + 1 ) % CONF_WATCHASYNC_BUFFERSIZE;
#else // def CONF_WATCHASYNC_SENDEND
    uint8_t temp = ( clock_get_time() / CONF_WATCHASYNC_RESOLUTION ) % CONF_WATCHASYNC_BUFFERSIZE;
#endif // def CONF_WATCHASYNC_SENDEND
#else // CONF_WATCHASYNC_RESOLUTION > 1
#ifdef CONF_WATCHASYNC_SENDEND
    uint8_t temp = ( clock_get_time() + 1) % CONF_WATCHASYNC_BUFFERSIZE;
#else // def CONF_WATCHASYNC_SENDEND
    uint8_t temp = clock_get_time() % CONF_WATCHASYNC_BUFFERSIZE;
#endif // def CONF_WATCHASYNC_SENDEND
#endif // CONF_WATCHASYNC_RESOLUTION > 1
    for (wa_buf = (temp + 1) % CONF_WATCHASYNC_BUFFERSIZE; wa_buf != temp; wa_buf = (wa_buf + 1) % CONF_WATCHASYNC_BUFFERSIZE)
    {
      for (wa_bufpin = 0; wa_bufpin < WATCHASYNC_PINCOUNT; wa_bufpin ++)
      {
        if (wa_buffer[wa_buf].pin[wa_bufpin] != 0)  // is there any data?
        {
          sendmessage();  // send the data found
	  return;
        }
      }
    }
#else // CONF_WATCHASYNC_SUMMARIZE
    if (wa_sendstate == 2) // Message not sent successfully
    {
      WATCHASYNC_DEBUG ("error, again please...\n"); 
      sendmessage();   // resend current event
    } else // sendstate == 0 => Idle  // Previous send has been succesfull, send next event if any
    {
      if (wa_buffer_left != wa_buffer_right) // there is something in the buffer
      {
        wa_buffer_left = ((wa_buffer_left + 1) % CONF_WATCHASYNC_BUFFERSIZE); // calculate next place in buffer
        WATCHASYNC_DEBUG ("starting transmission: L: %u R: %u Pin: %u\n", wa_buffer_left, wa_buffer_right, wa_buffer[wa_buffer_left].pin); 
        sendmessage();  // send the new event
      }
    }
#endif // CONF_WATCHASYNC_SUMMARIZE
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
