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
#include "config.h"
#include "services/dmx-storage/dmx_storage.h"
#ifndef DMX_USE_USART
#define DMX_USE_USART 0
#endif
#define USE_USART DMX_USE_USART
#define BAUD 250000
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init_8N2()

volatile uint8_t dmx_index;
volatile uint8_t dmx_txlen;

/**
 * Init DMX
 */
void
dmx_init(void)
{
  /* Initialize the usart module */
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
  uint8_t sreg = SREG; cli();
  /* Enable transmitter */
  PIN_SET(DMX_RS485EN); /* pull RS485EN pin high */

  /* Send RESET */
  PIN_CLEAR(DMX_RS485TX); /* pull TX pin low */
  _delay_us(88);
  /* End of RESET; Send MARK AFTER RESET */
  PIN_SET(DMX_RS485TX); /* pull TX pin high */
  _delay_us(8);

  /** Start a new dmx packet */
  /* Enable USART */
  usart(UCSR,B) = _BV(usart(TXEN));

  /* reset Transmit Complete flag */
  usart(UCSR,A) |= _BV(usart(TXC));

  /* Send Startbyte (not always 0!) */
  usart(UDR) = 0;

  /* Enable USART interrupt*/
  usart(UCSR,B) |= _BV(usart(TXCIE));
  SREG = sreg; sei();
}

void
dmx_tx_stop(void)
{
  uint8_t sreg = SREG; cli();

  /* Disable USART */
  usart(UCSR,B) = 0;

  /* Disable transmitter */
  PIN_CLEAR(DMX_RS485EN);

  SREG = sreg; sei();
  dmx_index = 0;  /* reset output channel index */
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

SIGNAL(usart(USART,_TX_vect))
{
  /* Send the rest */
  if(dmx_index < dmx_txlen) {
    if(usart(UCSR,A) & _BV(usart(UDRE))) {
      usart(UDR) = get_dmx_channel(DMX_OUTPUT_UNIVERSE,dmx_index++);
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
