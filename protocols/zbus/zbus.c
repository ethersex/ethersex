/*
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "core/heartbeat.h"
#include "protocols/zbus/zbus_raw_net.h"
#include "protocols/zbus/zbus.h"

#ifndef ZBUS_USE_USART
#define ZBUS_USE_USART 0
#endif
#define USE_USART ZBUS_USE_USART
#define BAUD ZBUS_BAUDRATE
#include "core/usart.h"

#ifdef DEBUG_ZBUS
#  include "core/debug.h"
#  define ZBUS_DEBUG(str...) debug_printf ("zbusnet: " str)
#else
#  define ZBUS_DEBUG(...)    ((void) 0)
#endif

/* We generate our own usart init module, for our usart port */
generate_usart_init ()
static uint8_t send_escape_data = 0;
static uint8_t recv_escape_data = 0;
static uint8_t bus_blocked = 0;

static volatile zbus_index_t zbus_index;
volatile zbus_index_t zbus_txlen;
static volatile zbus_index_t zbus_rxlen;
#ifdef ZBUS_ECMD
uint16_t zbus_rx_frameerror;
uint16_t zbus_rx_overflow;
uint16_t zbus_rx_parityerror;
uint16_t zbus_rx_bufferfull;
uint16_t zbus_rx_count;
uint16_t zbus_tx_count;
#endif

static void __zbus_txstart (void);

void
zbus_txstart (zbus_index_t size)
{
  // FIXME
  if (zbus_txlen != 0 || zbus_rxlen != 0 || bus_blocked)
    return;			/* rx or tx in action or
				   new packet left in buffer
				   or somebody is talking on the line */
  zbus_index = 0;

  zbus_txlen = size;

  if (bus_blocked)
    return;
  __zbus_txstart ();
}

static void
__zbus_txstart (void)
{

  uint8_t sreg = SREG;
  cli ();
  bus_blocked = 3;

  /* enable transmitter and receiver as well as their interrupts */
  usart (UCSR, B) = _BV (usart (TXCIE)) | _BV (usart (TXEN));

  /* Enable transmitter */
#ifdef HAVE_ZBUS_RXTX_PIN
  PIN_SET (ZBUS_RXTX_PIN);
#endif

  /* reset tx interrupt flag */
  usart (UCSR, A) |= _BV (usart (TXC));

  /* Go! */
  SREG = sreg;

  /* Transmit Start sequence */
  send_escape_data = ZBUS_START;
  usart (UDR) = '\\';

#ifdef STATUSLED_ZBUS_TX_SUPPORT
  PIN_SET (STATUSLED_ZBUS_TX);
#endif
  ACTIVITY_LED_ZBUS_TX;

  return;
}


void
zbus_rxstart (void)
{
  if (zbus_txlen > 0)
    {
      return;
    }
  zbus_rxlen = 0;

  uint8_t sreg = SREG;
  cli ();

  /* disable transmitter, enable receiver (and rx interrupt) */
  usart (UCSR, B) = _BV (usart (RXCIE)) | _BV (usart (RXEN));

  /* Default is reciever enabled */
#ifdef HAVE_ZBUS_RXTX_PIN
  PIN_CLEAR (ZBUS_RXTX_PIN);
#endif

  SREG = sreg;
}


static void
zbus_rxstop (void)
{
  uint8_t sreg = SREG;
  cli ();

  /* completely disable usart */
  usart (UCSR, B) = 0;

  SREG = sreg;
}


zbus_index_t
zbus_rxfinish (void)
{
  return zbus_rxlen;
}

void
zbus_core_init (void)
{
  /* Initialize the usart module */
  usart_init ();

  /* Enable RX/TX Swtich as Output */
#ifdef HAVE_ZBUS_RXTX_PIN
  DDR_CONFIG_OUT (ZBUS_RXTX_PIN);
#endif

  /* clear the buffers */
  zbus_txlen = 0;
  zbus_rxlen = 0;
  zbus_index = 0;

  zbus_rxstart ();
}

void
zbus_core_periodic (void)
{
  if (bus_blocked)
    if (--bus_blocked == 0 && zbus_txlen > 0)
      __zbus_txstart ();
}




ISR (usart (USART, _TX_vect))
{

#ifdef ZBUS_DEBUG
  ZBUS_DEBUG ("send data: %s\n", uip_appdata);
#endif

  /* If there's a carry byte, send it! */
  if (send_escape_data)
    {
#ifdef ZBUS_ECMD
      zbus_tx_count++;
#endif
      usart (UDR) = send_escape_data;
      send_escape_data = 0;
    }

  /* Otherwise send data from send context, if any is left. */
  else if (zbus_txlen && zbus_index < zbus_txlen)
    {
      if (zbus_buf[zbus_index] == '\\')
	{
	  /* We need to quote the character. */
	  send_escape_data = zbus_buf[zbus_index];
#ifdef ZBUS_ECMD
	  zbus_tx_count++;
#endif
	  usart (UDR) = '\\';
	}
      else
	{
	  /* No quoting needed, just send it. */
#ifdef ZBUS_ECMD
	  zbus_tx_count++;
#endif
	  usart (UDR) = zbus_buf[zbus_index];
	}

      zbus_index++;
      bus_blocked = 3;
    }

  /* If send_ctx contains data, but every byte has been sent over the
     wires, send a stop condition. */
  else if (zbus_txlen)
    {
      zbus_txlen = 0;		/* mark buffer as empty. */
      uip_buf_unlock ();

      /* Generate the stop condition. */
      send_escape_data = ZBUS_STOP;
#ifdef ZBUS_ECMD
      zbus_tx_count++;
#endif
      usart (UDR) = '\\';
    }




  /* Nothing to do, disable transmitter and TX LED. */
  else
    {
      bus_blocked = 0;
#ifdef STATUSLED_ZBUS_TX_SUPPORT
      PIN_CLEAR (STATUSLED_ZBUS_TX);
#endif
      zbus_txlen = 0;
      zbus_rxstart ();
    }
}

ISR (usart (USART, _RX_vect))
{
  /* Ignore errors */
  uint8_t flags = usart (UCSR, A);
  if (flags & (_BV (usart (FE)) | _BV (usart (DOR)) | _BV (usart (UPE))))
    {

#ifdef ZBUS_DEBUG
      ZBUS_DEBUG ("received data: %s\n", uip_appdata);
#endif


#ifdef ZBUS_ECMD
      if (flags & _BV (usart (FE)))
	zbus_rx_frameerror++;
      if (flags & _BV (usart (DOR)))
	zbus_rx_overflow++;
      if (flags & _BV (usart (UPE)))
	zbus_rx_parityerror++;
#endif
      flags = usart (UDR);	/* dummy read */
      return;
    }
  uint8_t data = usart (UDR);
#ifdef ZBUS_ECMD
  zbus_rx_count++;
#endif


  /* Old data is not read by application, ignore message */
  if (zbus_rxlen != 0)
    return;

  /* Don't accept incoming message if we're sending and sharing
     send and receive buffer. */
  if (zbus_txlen != 0)
    return;

  if (recv_escape_data)
    {
      recv_escape_data = 0;

      if (data == ZBUS_START)
	{
	  if (uip_buf_lock ())
	    return;		/* lock of buffer failed, ignore packet */

	  zbus_index = 0;
	  bus_blocked = 3;

	}

      else if (data == ZBUS_STOP)
	{
	  /* Only if there was a start condition before */
	  if (bus_blocked)
	    {
	      zbus_rxstop ();
	      zbus_rxlen = zbus_index;
	    }
#ifdef STATUSLED_ZBUS_RX_SUPPORT
	  PIN_CLEAR (STATUSLED_ZBUS_RX);
#endif

	  /* force bus free even if we didn't catch the start condition. */
	  bus_blocked = 0;
	  if (zbus_txlen > 0)
	    __zbus_txstart ();
	}

      else if (data == '\\')
	goto append_data;
    }

  else if (data == '\\')
    {

      recv_escape_data = 1;
#ifdef STATUSLED_ZBUS_RX_SUPPORT
      PIN_SET (STATUSLED_ZBUS_RX);
#endif
      ACTIVITY_LED_ZBUS_RX;
    }
  else
    {
    append_data:
      /* Not enough space in buffer */
      if (zbus_index >= ZBUS_BUFFER_LEN)
	{
#ifdef ZBUS_ECMD
	  zbus_rx_bufferfull++;
#endif
	  return;
	}

      /* If bus is not blocked we aren't on an message */
      if (!bus_blocked)
	return;

      bus_blocked = 3;
      zbus_buf[zbus_index] = data;
      zbus_index++;
    }
}

/*
  -- Ethersex META --
  header(protocols/zbus/zbus.h)
  init(zbus_core_init)
  timer(10, zbus_core_periodic())
*/
