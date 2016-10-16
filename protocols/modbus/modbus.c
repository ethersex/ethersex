/*
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
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"

#include "modbus_net.h"
#include "modbus_state.h"
#include "modbus.h"
#include "modbus_client.h"


#define USE_USART MODBUS_USE_USART
#define BAUD MODBUS_BAUDRATE
#include "core/usart.h"
#include "pinning.c"

#ifdef MODBUS_CLIENT_SUPPORT
struct modbus_connection_state_t modbus_client_state;
#endif

#define MODBUS_USART_CONFIG_8N1	1
#define MODBUS_USART_CONFIG_8E1	2

/* We generate our own usart init module, for our usart port */
#if MODBUS_USART_CONFIG == MODBUS_USART_CONFIG_8N1
        generate_usart_init()
#elif MODBUS_USART_CONFIG == MODBUS_USART_CONFIG_8E1
        generate_usart_init_8E1()
#else
        #error "MODBUS_USART_CONFIG not correctly defined"
#endif

volatile struct modbus_buffer modbus_data;

uint8_t modbus_recv_timer = 0;
int16_t *modbus_recv_len_ptr = NULL;
uint8_t modbus_last_address;

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
#if !RS485_HAVE_TE
  #error no RS485 transmit enable pin for MODBUS defined
#endif

  RS485_TE_SETUP;             // configure RS485 transmit enable as output
  RS485_DISABLE_TX;           // disable RS485 transmitter
  usart_init();               // initialize the usart module

  modbus_data.len = 0;
  modbus_data.sent = 0;
  modbus_data.crc_len = 0;

#ifdef MODBUS_CLIENT_SUPPORT
    modbus_client_state.len = 0;
#endif

}

void
modbus_periodic(void)
{
  if (modbus_recv_timer == 0) return;
  modbus_recv_timer--;

  if (!modbus_recv_len_ptr) {
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
      int16_t recv_len;
      modbus_client_process(modbus_client_state.data, modbus_client_state.len,
                            &recv_len);
      if (recv_len) {
        RS485_ENABLE_TX;

        modbus_data.data = modbus_client_state.data;
        modbus_data.len = recv_len; 

        /* Enable the tx interrupt and send the first character */
        modbus_data.sent = 1;
        usart(UCSR,B) |= _BV(usart(TXCIE));
        usart(UDR) = modbus_client_state.data[0];
      }
      modbus_client_state.len = 0;
    }
#endif
    return;
  }
  if (modbus_recv_timer != 0) return;
  *modbus_recv_len_ptr = modbus_data.len;

  if (*modbus_recv_len_ptr < 2)
      *modbus_recv_len_ptr = -1;

  if (modbus_data.data[0] != modbus_last_address) {
      *modbus_recv_len_ptr = -1;
  }
  modbus_recv_len_ptr = NULL;
}

uint8_t
modbus_rxstart(uint8_t *data, uint8_t len, int16_t *recv_len) {
#ifdef MODBUS_CLIENT_SUPPORT
  if (data[0] == MODBUS_ADDRESS || data[0] == MODBUS_BROADCAST) {
    modbus_client_process(data, len, recv_len);
    if (data[0] == MODBUS_ADDRESS)
      return 1;
  }
#endif
  if (modbus_data.crc_len != 0) return 0; /* There is an packet on the way */

  modbus_last_address = *data;

  RS485_ENABLE_TX;

  modbus_recv_len_ptr = recv_len;

  modbus_data.crc = modbus_crc_calc(data, len);
  modbus_data.crc_len = 2;

  modbus_data.data = data;
  modbus_data.len = len;

  /* Enable the tx interrupt and send the first character */
  modbus_data.sent = 1;
  usart(UCSR,B) |= _BV(usart(TXCIE));
  usart(UDR) = data[0];

  return 1;
}

ISR(usart(USART,_TX_vect))
{
  if (modbus_data.sent < modbus_data.len) {
    usart(UDR) = modbus_data.data[modbus_data.sent++];
  } else if (modbus_data.crc_len != 0) {
    /* Send the crc checksum */
    usart(UDR) = modbus_data.crc >> (( 1 - (--modbus_data.crc_len)) * 8);
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));

    RS485_DISABLE_TX;

    /* No we are waiting for an answer */
    if (modbus_recv_len_ptr) {
      modbus_data.len = 0;
      modbus_recv_timer = 4;
    }
  }
}

ISR(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE))))
  {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
  uint8_t data = usart(UDR);


  if (!modbus_recv_len_ptr)
  {
#ifdef MODBUS_CLIENT_SUPPORT
    /* This byte is not answer to a modbus/TCP || ecmd modbus request */
    modbus_client_state.data[modbus_client_state.len++] = data;
    modbus_recv_timer = 2;
#endif
    return;
  }
  /* Is the buffer big enough */
  if (modbus_data.len >= MODBUS_BUFFER_LEN) return;

  modbus_data.data[modbus_data.len++] = data;

  modbus_recv_timer = 2;
}

/*
  -- Ethersex META --
  header(protocols/modbus/modbus.h)
  init(modbus_init)
  timer(1, modbus_periodic())
*/
