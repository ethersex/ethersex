/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "config.h"
#include "services/dmx-storage/dmx_storage.h"
#ifndef DMX_USE_USART
#define DMX_USE_USART 0
#endif
#define USE_USART DMX_USE_USART
#define BAUD 250000
#include "core/usart.h"

/* generating private usart init */
generate_usart_init_8N2()

volatile uint8_t dmx_index;
volatile uint8_t dmx_txlen;

/**
 * Init DMX
 */
void
dmx_init(void)
{
  /* initialize the usart module */
#if (USE_USART == 0 && defined(HAVE_RS485TE_USART0))
  PIN_CLEAR(RS485TE_USART0);      // disable RS485 transmitter for usart 0
  DDR_CONFIG_OUT(RS485TE_USART0);
#elif (USE_USART == 1  && defined(HAVE_RS485TE_USART1))
  PIN_CLEAR(RS485TE_USART1);      // disable RS485 transmitter for usart 1
  DDR_CONFIG_OUT(RS485TE_USART1);
#else
  #warning no RS485 transmit enable pin for DMX defined
#endif

  usart_init();
  /* Clear the buffers */
  dmx_txlen = DMX_STORAGE_CHANNELS;
  dmx_index = 0;
}

/**
 * Send DMX-channels via USART
 */
void
dmx_tx_start(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
#if (USE_USART == 0)
#ifdef HAVE_RS485TE_USART0
    PIN_SET(RS485TE_USART0);          // enable RS485 transmitter for usart 0
#endif
    PIN_CLEAR(TXD0);                  // generate a break signal on usart 0
    _delay_us(88);
    PIN_SET(TXD0);                    // make after break
    _delay_us(8);
#elif (USE_USART == 1)
#ifdef HAVE_RS485TE_USART1
    PIN_SET(RS485TE_USART1);          // enable RS485 transmitter for usart 1
#endif
    PIN_CLEAR(TXD1);                  // generate a break signal on usart 1
    _delay_us(88);
    PIN_SET(TXD1);                    // make after break
    _delay_us(8);
#endif

    /* start a new dmx packet */
    usart(UCSR,B) = _BV(usart(TXEN));   // enable usart
    usart(UCSR,A) |= _BV(usart(TXC));   // reset transmit complete flag
    usart(UDR) = 0;                     // send startbyte (not always 0!)
    usart(UCSR,B) |= _BV(usart(TXCIE)); // enable usart interrupt
  }
}

void
dmx_tx_stop(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    usart(UCSR, B) = 0;               // disable usart

#if (USE_USART == 0 && defined(HAVE_RS485TE_USART0))
    PIN_CLEAR(RS485TE_USART0);        // disable RS485 transmitter for usart 0
#elif (USE_USART == 1  && defined(HAVE_RS485TE_USART1))
    PIN_CLEAR(RS485TE_USART1);        // disable RS485 transmitter for usart 1
#endif
    dmx_index = 0;                    // reset output channel index
  }
}

/**
 * Send DMX-packet
 */
void
dmx_periodic(void)
{
  wdt_kick();
  if(dmx_index == 0) {
    dmx_tx_start();
  }
}

ISR(usart(USART,_TX_vect))
{
  /* Send the rest */
  if(dmx_index < dmx_txlen) {
    if(usart(UCSR, A) & _BV(usart(UDRE))) {
      usart(UDR) = get_dmx_channel(DMX_OUTPUT_UNIVERSE, dmx_index++);
    }
  } else
    dmx_tx_stop();
}

/*
  -- Ethersex META --
  header(protocols/dmx/dmx.h)
  init(dmx_init)
  timer(2, dmx_periodic())
*/
