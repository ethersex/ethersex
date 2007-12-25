/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#include <util/delay.h>
#include "../bit-macros.h"
#include "../config.h"
#include "../syslog/syslog.h"
#include "zbus.h"



static volatile uint8_t send_escape_data = 0;
static volatile uint8_t recv_escape_data = 0;
static volatile uint8_t bus_blocked = 0;

static volatile zbus_send_byte_callback_t callback = NULL;
static volatile void *callback_ctx = NULL;

#ifdef ENC28J60_SUPPORT
static volatile uint8_t recv_buffer[ZBUS_RECV_BUFFER];
#endif

static volatile struct zbus_ctx recv_ctx;
static volatile struct zbus_ctx send_ctx;

static uint8_t
zbus_send_data_cb(void **ctx) 
{
  if ( send_ctx.len == 0 ) 
    zbus_tx_finish();
  else {
    char data = send_ctx.data[send_ctx.offset];
    send_ctx.offset++;
    if (send_ctx.offset >= send_ctx.len)
      send_ctx.len = 0;
    return data;
  }


}

uint8_t 
zbus_send_data(uint8_t *data, uint16_t len)
{
  if (send_ctx.len == 0) {
    send_ctx.data = data;
    send_ctx.len = len;
    send_ctx.offset = 0;
    zbus_tx_start(zbus_send_data_cb, 0);
    return 1;
  }
  return 0;
}

struct zbus_ctx *
zbus_rxfinish(void) 
{
  if (recv_ctx.len != 0)
    return &recv_ctx;
  return NULL;
}

void
zbus_core_init(void)
{
    /* set baud rate */
    _UBRRH_UART0 = HI8(ZBUS_UART_UBRR);
    _UBRRL_UART0 = LO8(ZBUS_UART_UBRR);

    /* set mode */
#ifdef URSEL
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01) | _BV(URSEL);
#else
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);
#endif

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_RXCIE_UART0) | _BV(_TXEN_UART0) | _BV(_RXEN_UART0);

    /* Enable RX/TX Swtich as Output */
    RXTX_DDR |= _BV(RXTX_PIN);
    /* Default is reciever enabled*/
    RXTX_PORT &= ~_BV(RXTX_PIN);

#ifdef ZBUS_BLINK_PORT
  ZBUS_BLINK_DDR |= ZBUS_RX_PIN | ZBUS_TX_PIN;
#endif
    
    /* clear the buffers */
    send_ctx.len = 0;
    recv_ctx.len = 0;
#ifdef ENC28J60_SUPPORT
    recv_ctx.data = (uint8_t *)recv_buffer;
#else
    recv_ctx.data = (uint8_t *)uip_buf;
#endif
}

void
zbus_core_periodic(void)
{
  if(bus_blocked)
    bus_blocked--;
}


uint8_t
zbus_tx_start(zbus_send_byte_callback_t cb, void *ctx) 
{
  /* Return if there is an sending process */
  if (callback) return 0;

  /* Enable transmitter */
  RXTX_PORT |= _BV(RXTX_PIN);

  /* Install send byte callback */
  callback = cb;
  callback_ctx = ctx;

  _delay_ms(1);
  
  /* Transmit Start sequence */
  send_escape_data = ZBUS_START;
  _UDR_UART0 = '\\';

  /* Enable buffer empty interrupt */
  _UCSRB_UART0 |= _BV(UDRIE0); 

#ifdef ZBUS_BLINK_PORT
  ZBUS_BLINK_PORT |= ZBUS_TX_PIN;
#endif

  return 1;
}

void  
zbus_tx_finish(void) 
{
  callback = NULL;
#ifdef ZBUS_BLINK_PORT
  ZBUS_BLINK_PORT &= ~ZBUS_TX_PIN;
#endif
}

SIGNAL(USART0_UDRE_vect)
{
  _delay_ms(1);

  if (send_escape_data) {
    _UDR_UART0 = send_escape_data;
    send_escape_data = 0;
    if (callback == NULL) {
      /* Wait for completion */
      while (!(_UCSRA_UART0 & _BV(_UDRE_UART0)));
      /* Disable this interrupt */
      _UCSRB_UART0 &= ~_BV(UDRIE0); 
      _delay_ms(1);
      /* Disable transmitter */
      RXTX_PORT &= ~_BV(RXTX_PIN);
    }
  } else {
    if (callback) {
      send_escape_data = callback((void *)callback_ctx);
      if (callback) {
        if (send_escape_data == '\\') 
          _UDR_UART0 = '\\';
        else {
          _UDR_UART0 = send_escape_data;
          send_escape_data = 0;
        }
      } else {
        /* Send Packet end */
        _UDR_UART0 = '\\';
        send_escape_data = ZBUS_STOP;
      }
    }
  }
}

SIGNAL(USART0_RX_vect)
{
  /* Ignore errors */
  if ((_UCSRA_UART0 & _BV(DOR0)) || (_UCSRA_UART0 & _BV(FE0))) {
    uint8_t v = _UDR_UART0;
    return; 
  }
  uint8_t data = _UDR_UART0;

  /* Old data is not read by application, ignore message */
  if (recv_ctx.len != 0) return;

  if (data == '\\') 
    recv_escape_data = 1;
  else {
    if (recv_escape_data){
      if (data == ZBUS_START) {
        recv_ctx.offset = 0;
        bus_blocked = 3;
#ifdef ZBUS_BLINK_PORT
        ZBUS_BLINK_PORT |= ZBUS_RX_PIN;
#endif
      }
      else if (data == ZBUS_STOP) {
        /* Only if there was a start condition before */
        if (bus_blocked) {
#ifdef ZBUS_BLINK_PORT
          ZBUS_BLINK_PORT &= ~ZBUS_RX_PIN;
#endif
          recv_ctx.len = recv_ctx.offset;
          bus_blocked = 0;
        }
      }
      else if (data == '\\') {
        recv_escape_data = 0;
        goto append_data;
      }
      recv_escape_data = 0;
    } else {
append_data:
      /* Not enough space in buffer */
      if (recv_ctx.offset >= (ZBUS_RECV_BUFFER)) return;
      /* If bus is not blocked we aren't on an message */
      if (!bus_blocked) return;

      recv_ctx.data[recv_ctx.offset] = data;
      recv_ctx.offset++;
    }
  }
}
