/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "../bit-macros.h"
#include "../usart.h"
#include "../eeprom.h"
#include "../config.h"
#include "../syslog/syslog.h"
#include "../crypto/encrypt-llh.h"
#include "zbus.h"

static uint8_t send_escape_data = 0;
static uint8_t recv_escape_data = 0;
static uint8_t bus_blocked = 0;

#ifdef ENC28J60_SUPPORT
static uint8_t recv_buffer[ZBUS_RECV_BUFFER];
#endif

static struct zbus_ctx recv_ctx;
static struct zbus_ctx send_ctx;

static uint8_t zbus_txstart(void);

uint8_t 
zbus_send_data(uint8_t *data, uint16_t len)
{
  if (send_ctx.len == 0) {
#if defined(SKIPJACK_SUPPORT) && defined(ENC28J60_SUPPORT)
    if (recv_ctx.len != 0 || len > ZBUS_RECV_BUFFER) 
      return 0;			/* we mustn't use recv buffer yet,
				   nor may we overflow it. */

    /* we need to duplicate since we mustn't encrypt uip_buf! */
    send_ctx.data = recv_buffer;
    memmove (recv_buffer, data, len);
#else
    send_ctx.data = data;
#endif
    send_ctx.len = len;
    send_ctx.offset = 0;
#ifdef SKIPJACK_SUPPORT
    zbus_encrypt(send_ctx.data, &send_ctx.len);
#endif
    zbus_txstart();
    return 1;
  }
  return 0;
}


void
zbus_rxstart (void)
{
  if(send_ctx.len > 0){
    zbus_txstart();
    return;
  }
  uint8_t sreg = SREG; cli();

  /* disable transmitter, enable receiver (and rx interrupt) */
  _UCSRB_UART0 = _BV(_RXCIE_UART0) | _BV(_RXEN_UART0);

  /* Default is reciever enabled*/
  PIN_CLEAR(ZBUS_RXTX_PIN);

  SREG = sreg;
}


static void
zbus_rxstop (void)
{
  uint8_t sreg = SREG; cli();

  /* completely disable usart */
  _UCSRB_UART0 = 0;

  SREG = sreg;
}


struct zbus_ctx *
zbus_rxfinish(void) 
{
  if (recv_ctx.len != 0) {
#ifdef SKIPJACK_SUPPORT
    zbus_decrypt(recv_ctx.data, &recv_ctx.len);
#endif
    return (struct zbus_ctx *) (&recv_ctx);
  }
  return NULL;
}

void
zbus_core_init(void)
{
    /* Initialize the usart module */
    usart_init();

    /* Enable RX/TX Swtich as Output */
    DDR_CONFIG_OUT(ZBUS_RXTX_PIN);

#ifdef HAVE_ZBUS_RX_PIN
    DDR_CONFIG_OUT(ZBUS_RX_PIN);
#endif
#ifdef HAVE_ZBUS_TX_PIN
    DDR_CONFIG_OUT(ZBUS_TX_PIN);
#endif

    /* clear the buffers */
    send_ctx.len = 0;
    recv_ctx.len = 0;
#ifdef ENC28J60_SUPPORT
    recv_ctx.data = (uint8_t *)recv_buffer;
#else
    recv_ctx.data = (uint8_t *)uip_buf;
#endif

    zbus_rxstart ();
}

void
zbus_core_periodic(void)
{
  if(bus_blocked)
    if(--bus_blocked == 0 && send_ctx.len > 0)
      zbus_txstart();
}


static uint8_t
zbus_txstart(void) 
{
  if(bus_blocked)
    return 0;
  uint8_t sreg = SREG; cli();

  /* enable transmitter and receiver as well as their interrupts */
  _UCSRB_UART0 = _BV(_TXCIE_UART0) | _BV(_TXEN_UART0);

  /* Enable transmitter */
  PIN_SET(ZBUS_RXTX_PIN);

  /* reset tx interrupt flag */
  _UCSRA_UART0 |= _BV(_TXC_UART0);

  /* Go! */
  SREG = sreg;

  /* Transmit Start sequence */
  send_escape_data = ZBUS_START;
  _UDR_UART0 = '\\';

#ifdef HAVE_ZBUS_TX_PIN
  PIN_SET(ZBUS_TX_PIN);
#endif

  return 1;
}


SIGNAL(USART0_TX_vect)
{
  /* If there's a carry byte, send it! */
  if (send_escape_data) {
    _UDR_UART0 = send_escape_data;
    send_escape_data = 0;
  }

  /* Otherwise send data from send context, if any is left. */
  else if (send_ctx.len && send_ctx.offset < send_ctx.len) {
    if (send_ctx.data[send_ctx.offset] == '\\') {
      /* We need to quote the character. */
      send_escape_data = send_ctx.data[send_ctx.offset];
      _UDR_UART0 = '\\';
    }
    else {
      /* No quoting needed, just send it. */
      _UDR_UART0 = send_ctx.data[send_ctx.offset];
    }

    send_ctx.offset ++;
  }

  /* If send_ctx contains data, but every byte has been sent over the
     wires, send a stop condition. */
  else if (send_ctx.len) {
    send_ctx.len = 0;		/* mark buffer as empty. */

    /* Generate the stop condition. */
    send_escape_data = ZBUS_STOP;
    _UDR_UART0 = '\\';
  }

  /* Nothing to do, disable transmitter and TX LED. */
  else {
#ifdef HAVE_ZBUS_TX_PIN
    PIN_CLEAR(ZBUS_TX_PIN);
#endif
    
    zbus_rxstart ();
  }
}

SIGNAL(USART0_RX_vect)
{
  /* Ignore errors */
  if ((_UCSRA_UART0 & _BV(DOR0)) || (_UCSRA_UART0 & _BV(FE0))) {
    uint8_t v = _UDR_UART0;
    (void) v;
    return; 
  }
  uint8_t data = _UDR_UART0;

  /* Old data is not read by application, ignore message */
  if (recv_ctx.len != 0) return;

#if defined(SKIPJACK_SUPPORT) && defined(ENC28J60_SUPPORT)
  /* Don't accept incoming message if we're sending and sharing
     send and receive buffer. */
  if (send_ctx.len != 0) return;
#endif

  if (recv_escape_data) {
    recv_escape_data = 0;

    if (data == ZBUS_START) {
      recv_ctx.offset = 0;
      bus_blocked = 3;
#ifdef HAVE_ZBUS_RX_PIN
      PIN_SET(ZBUS_RX_PIN);
#endif
    }

    else if (data == ZBUS_STOP) {
      /* Only if there was a start condition before */
      if (bus_blocked) {
	zbus_rxstop ();

#ifdef HAVE_ZBUS_RX_PIN
        PIN_CLEAR(ZBUS_RX_PIN);
#endif
	recv_ctx.len = recv_ctx.offset;
      }

      /* force bus free even if we didn't catch the start condition. */
      bus_blocked = 0;
      if(send_ctx.len > 0)
        zbus_txstart();
    }

    else if (data == '\\')
      goto append_data;
  } 

  else if (data == '\\') 
    recv_escape_data = 1;

  else {
  append_data:
    /* Not enough space in buffer */
    if (recv_ctx.offset >= (ZBUS_RECV_BUFFER))
      return;

    /* If bus is not blocked we aren't on an message */
    if (!bus_blocked)
      return;
      
    recv_ctx.data[recv_ctx.offset] = data;
      recv_ctx.offset++;
  }
}
