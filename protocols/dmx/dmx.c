/*
 * Copyright (c) 2012 by Frank Sautter <ethersix@sautter.com>
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

/*
 * for DMX timing specifications see
 * http://www.erwinrol.com/dmx512/ or
 * http://opendmx.net/index.php/DMX512-A
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

/* baudrate used for DMX data bytes */
#define BAUD 250000
/* baudrate to generate 176µs break signal at start of packet */
#define BAUD_BREAK 51000

#include "core/usart.h"
#include "pinning.c"

#define UBRR_DMX       (((F_CPU) + 8UL * (BAUD)) / (16UL * (BAUD)) -1UL)
#define UBRR_DMX_BREAK (((F_CPU) + 8UL * (BAUD_BREAK)) / \
                         (16UL * (BAUD_BREAK)) -1UL)

typedef enum {
  DMX_BREAK,
  DMX_START,
  DMX_DATA,
} dmx_tx_state_t;

static volatile uint16_t dmx_index = 0;
static volatile uint16_t dmx_txlen = DMX_STORAGE_CHANNELS;
static volatile dmx_tx_state_t dmx_tx_state = DMX_START;


/**
 * init DMX
 * after initialisation of IOs, all data transfer is done by ISRs without the
 * need for any delay loops
 */
void dmx_init(void) {

  dmx_tx_state = DMX_START;   // init state machine
                              // (SRAM is not reset on chip reset)

#if (USE_USART == 0)
  PIN_SET(TXD0);              // set usart tx pin high (mark)
  DDR_CONFIG_OUT(TXD0);       // configure usart tx pin as output
#elif (USE_USART == 1)
  PIN_SET(TXD1);              // set usart tx pin high (mark)
  DDR_CONFIG_OUT(TXD1);       // configure usart tx pin as output
#endif

  RS485_TE_SETUP;             // configure RS485 transmit enable as output
  RS485_ENABLE_TX;            // enable RS485 transmitter

#if !RS485_HAVE_TE
  #warning no RS485 transmit enable pin for DMX defined
#endif

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    usart(UBRR,H) = (UBRR_DMX_BREAK >> 8);
    usart(UBRR,L) = (UBRR_DMX_BREAK & 0xff);
    /* set mode 8N2: 8 bits, 2 stop, no parity, asynchronous usart
     * and set URSEL, if present, */
    usart(UCSR,C) =  _BV(usart(USBS)) | (3 << (usart(UCSZ,0))) | _BV_URSEL;
    USART_2X();
    /* transmitter enable, TX complete interrupt enable, UDR empty disable */
    usart(UCSR, B) = _BV(usart(TXEN)) | _BV(usart(TXCIE));

    /* start by sending a break signal */
    usart(UDR) = 0;
  }

}

/**
 * DMX interrupt service routines
 * DMX_BREAK: send a 176µs break signal
 * DMX_START: send a start byte with 250kbps
 * DMX_DATA:  send up to 511 bytes of DMX data with 250kbps
 *
 * how this all works:
 * - the usart is initialized with 8N2.
 *   baudrate is set to BAUD_BREAK achieve a break signal.
 *   transfer register is filled with a 0 byte.
 *   TXCIE (transmission complete) interrupt is enabled.
 * - after the break signal is completely sent TXCIE interrupt is triggered.
 *   baudrate is set to 250kbps
 *   the start of frame byte (0) is stored in the transfer register
 *   TXCIE interrupt is disabled
 *   UDRE (transmission register empty) is enabled
 * - all DMX data is sent in UDRE interrupt
 *   after the last byte UDRE interrupt is disabled and TXCIE enabled
 * - baudrate is set to BAUD_BREAK
 *   break signal is sent
 *
 * why so complicated?
 * to achieve maximum transmission rate and not to loose time having the next
 * byte not already prepared (only using TXCIE would lead to ~ 1-2 bittime
 * longer marks)
 * using the usart to generate the break signal makes delay loops unnecessary.
 * everything - except initialization- is done using interrupts.
 */
ISR(usart(USART, _TX_vect))
{
  switch (dmx_tx_state) {
    case DMX_BREAK:
      /* set break baudrate */
      usart(UBRR,H) = (UBRR_DMX_BREAK >> 8);
      usart(UBRR,L) = (UBRR_DMX_BREAK & 0xff);
      /* send break signal */
      usart(UDR) = 0;
      /* reset data pointer */
      dmx_index = 0;
      dmx_tx_state = DMX_START;
      break;

    case DMX_START:
      /* set normal DMX baudrate */
      usart(UBRR,H) = (UBRR_DMX >> 8);
      usart(UBRR,L) = (UBRR_DMX & 0xff);
      /* send start byte */
      usart(UDR) = 0;
      /* transmitter enable, TX complete interrupt disable, UDR empty enable */
      usart(UCSR, B) = _BV(usart(TXEN)) | _BV(usart(UDRE));
      dmx_tx_state = DMX_DATA;
      break;
    default:        // just to quiet compiler warning
      break;
    }
}

ISR(usart(USART, _UDRE_vect))
{
  /* send DMX data bytes */
  usart(UDR) = get_dmx_channel(DMX_OUTPUT_UNIVERSE, dmx_index++);

  /* restart if end of universe is reached */
  if (dmx_index >= dmx_txlen)
  {
    /* transmitter enable, TX complete interrupt enable, UDR empty disable */
    usart(UCSR, B) = _BV(usart(TXEN)) | _BV(usart(TXCIE));
    dmx_tx_state = DMX_BREAK;
  }
}

/*
 -- Ethersex META --
 header(protocols/dmx/dmx.h)
 init(dmx_init)
 */
