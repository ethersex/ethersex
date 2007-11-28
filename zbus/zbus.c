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
#include "../net/zbus_state.h"



static volatile uint8_t send_escape_data = 0;
static volatile uint8_t recv_escape_data = 0;
static volatile zbus_send_byte_callback_t callback = NULL;
static volatile void *callback_ctx = NULL;

static volatile struct uip_udp_conn *send_connection = NULL;
static volatile struct uip_udp_conn *recv_connection = NULL;

uint8_t
zbus_send_conn_data_cb(void **ctx) 
{
  if (! send_connection ) 
    zbus_tx_finish();
  else {
    char data = send_connection->
      appstate.zbus.buffer[send_connection->appstate.zbus.offset];
    send_connection->appstate.zbus.offset++;
    if (send_connection->appstate.zbus.offset 
        >= send_connection->appstate.zbus.buffer_len)
      send_connection = NULL;
    return data;
  }


}

uint8_t
zbus_send_conn_data(struct uip_udp_conn *conn) 
{
  if (!send_connection) {
    send_connection = conn;
    conn->appstate.zbus.offset = 0;
    zbus_tx_start(zbus_send_conn_data_cb, NULL);
    return 1;
  }
  return 0;
}


void
zbus_core_init(struct uip_udp_conn *recv_conn)
{
    /* set baud rate */
    _UBRRH_UART0 = HI8(ZBUS_UART_UBRR);
    _UBRRL_UART0 = LO8(ZBUS_UART_UBRR);

    /* set mode */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_RXCIE_UART0) | _BV(_TXEN_UART0) | _BV(_RXEN_UART0);

    /* Enable RX/TX Swtich as Output */
    RXTX_DDR |= _BV(RXTX_PIN);
    /* Default is reciever enabled*/
    RXTX_PORT &= ~_BV(RXTX_PIN);
    
    /* copy the recieve connection */
    recv_connection = recv_conn;

    /* Set the recv Buffer to invalid */
    recv_connection->appstate.zbus.state &= ~ZBUS_STATE_RECIEVED;

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
  
  /* Transmit Start sequence */
  send_escape_data = ZBUS_START;
  _UDR_UART0 = '\\';

  /* Enable buffer empty interrupt */
  _UCSRB_UART0 |= _BV(UDRIE0); 

  return 1;
}

void  
zbus_tx_finish(void) 
{
  callback = NULL;
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
  if (recv_connection->appstate.zbus.state & ZBUS_STATE_RECIEVED) return;

  if (data == '\\') 
    recv_escape_data = 1;
  else {
    if (recv_escape_data){
      if (data == ZBUS_START) {
        recv_connection->appstate.zbus.buffer_len = 0;
      }
      else if (data == ZBUS_STOP) 
        recv_connection->appstate.zbus.state |= ZBUS_STATE_RECIEVED;
      else if (data == '\\') {
        recv_escape_data = 0;
        goto append_data;
      }
      recv_escape_data = 0;
    } else {
append_data:
      /* Not enough space in buffer */
      if (recv_connection->appstate.zbus.buffer_len >= ZBUS_BUFFER_LEN) return;
      recv_connection->appstate.zbus
        .buffer[recv_connection->appstate.zbus.buffer_len++] = data;
    }
  }
}
