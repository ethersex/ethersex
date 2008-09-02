/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include <util/crc16.h>
#include <string.h>
#include "../eeprom.h"
#include "../net/yport_net.h"
#include "../bit-macros.h"
#include "../config.h"
#include "../net/modbus_net.h"
#include "../net/modbus_state.h"
#include "modbus.h"
#include "modbus_client.h"

#include "../pinning.c"

#ifdef MODBUS_SUPPORT

#define USE_USART MODBUS_USE_USART
#include "../usart.h"

#ifdef MODBUS_CLIENT_SUPPORT
struct modbus_connection_state_t modbus_client_state;
#endif


/* We generate our own usart init module, for our usart port */
generate_usart_init(MODBUS_UART_UBRR)

volatile struct modbus_buffer modbus_send_buffer;
uint8_t modbus_recv_timer = 0;

uint16_t 
modbus_crc_calc(uint8_t *data, uint8_t len) 
{
  uint16_t crc = 0xffff;
  uint8_t i = 0;
  while(i < len)
    crc = _crc16_update(crc, data[i++]);
  return crc;
}

void
modbus_init(void)
{
    /* Initialize the usart module */
    usart_init();

    /* Enable RX/TX Swtich as Output */
    DDR_CONFIG_OUT(MODBUS_TX);
    PIN_CLEAR(MODBUS_TX);

    modbus_send_buffer.len = 0;
    modbus_send_buffer.sent = 0;
    modbus_send_buffer.crc_len = 0;

#ifdef MODBUS_CLIENT_SUPPORT
    modbus_client_state.len = 0;
#endif
    
}

void
modbus_periodic(void)
{
  if (modbus_recv_timer == 0) return;
  modbus_recv_timer--;

  if (!modbus_conn) {
#ifdef MODBUS_CLIENT_SUPPORT
    if (modbus_recv_timer == 0) {
      /* check the crc */
      uint16_t crc = modbus_crc_calc(modbus_client_state.data, modbus_client_state.len - 2);
      uint16_t crc_recv = 
        ((modbus_client_state.data[modbus_client_state.len - 1])  << 8) 
        | modbus_client_state.data[modbus_client_state.len - 2];
      if (crc != crc_recv) {
        modbus_client_state.len = 0;
        return;
      }
      /* See if we are the receiver */
      if (!(modbus_client_state.data[0] == MODBUS_ADDRESS 
          || modbus_client_state.data[0] == MODBUS_BROADCAST))
        return;
      modbus_client_state.len -= 2;
      /* A message for our own modbus stack */
      modbus_client_process(modbus_client_state.data, modbus_client_state.len,
                            &modbus_client_state);
      if (modbus_client_state.len) {
        PIN_SET(MODBUS_TX);
        modbus_send_buffer.data = modbus_client_state.data;
        modbus_send_buffer.len = modbus_client_state.len;

        /* Enable the tx interrupt and send the first character */
        modbus_send_buffer.sent = 1;
        usart(UCSR,B) |= _BV(usart(TXCIE));
        usart(UDR) = modbus_client_state.data[0];
      }
      modbus_client_state.len = 0;
    }
#endif
    return;
  }
  if (modbus_recv_timer != 0) return;
  modbus_conn->appstate.modbus.waiting_for_answer = 0;
  modbus_conn->appstate.modbus.new_data = 1;
  modbus_conn = NULL;
}

uint8_t 
modbus_rxstart(uint8_t *data, uint8_t len) {
#ifdef MODBUS_CLIENT_SUPPORT
  if (data[0] == MODBUS_ADDRESS || data[0] == MODBUS_BROADCAST) {
    modbus_client_process(data, len, (void *)&modbus_conn->appstate.modbus);
    if (data[0] == MODBUS_ADDRESS) 
      return 1;
  }
#endif
  if (modbus_send_buffer.crc_len != 0) return 0; /* There is an packet on the way */

  /* enable the transmitter */
  PIN_SET(MODBUS_TX);

  modbus_send_buffer.crc = modbus_crc_calc(data, len);
  modbus_send_buffer.crc_len = 2;

  modbus_send_buffer.data = data;
  modbus_send_buffer.len = len;

  /* Enable the tx interrupt and send the first character */
  modbus_send_buffer.sent = 1;
  usart(UCSR,B) |= _BV(usart(TXCIE));
  usart(UDR) = data[0];

  return 1;
}

SIGNAL(usart(USART,_TX_vect))
{
  if (modbus_send_buffer.sent < modbus_send_buffer.len) {
    usart(UDR) = modbus_send_buffer.data[modbus_send_buffer.sent++];
  } else if (modbus_send_buffer.crc_len != 0) {
    /* Send the crc checksum */
    usart(UDR) = modbus_send_buffer.crc >> (( 1 - (--modbus_send_buffer.crc_len)) * 8);
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
    /* Disable the transmitter */
    PIN_CLEAR(MODBUS_TX);
    /* free the modbus_conn */
    if (modbus_conn) {
      modbus_conn->appstate.modbus.must_send = 0;
      modbus_conn->appstate.modbus.waiting_for_answer = 1;
      modbus_conn->appstate.modbus.len = 0;
      modbus_recv_timer = 4;
    }
  }
}

SIGNAL(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return; 
  }
  uint8_t data = usart(UDR);

  
  if (!modbus_conn) {
#ifdef MODBUS_CLIENT_SUPPORT
    /* This byte is not answer to a modbus/TCP || ecmd modbus request */
    modbus_client_state.data[modbus_client_state.len++] = data;
    modbus_recv_timer = 2;
#endif
    return;
  }
  /* Is the buffer big enough */
  if (modbus_conn->appstate.modbus.len >= MODBUS_BUFFER_LEN) return;

  modbus_conn->appstate.modbus.data[modbus_conn->appstate.modbus.len++] = data;

  modbus_recv_timer = 2;
}

#endif
