/*
 * Copyright (c) 2011 by Danny Baumann <dannybaumann@web.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3)
 * as published by the Free Software Foundation.
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

#include <avr/interrupt.h>
#include "config.h"
#include "ems.h"

#define BAUD 9600

/*
 * EMS theory:
 *
 * Polling: master sends (address | 0x80)
 * Answer:
 * 0) nothing to send: <addr> <break>
 * 1) Broadcast: <addr> 0x0 <data> ... <break>
 * 2) Send without request: <addr> <dest> <data> ... <crc> <break>
 * 3) Send with request: <addr> <dest | 0x80> <data> ... <crc> <break>
 *
 * -> send <addr> <txdata> <break>
 *
 * after tx byte compare with rx
 * if mismatch -> abort with <break>
 */

/* test data to send: "88 02 00 06" */

#define STATE_RX                 0
#define STATE_TX_ADDR            1
#define STATE_TX_ADDR_WAIT_ECHO  2
#define STATE_TX_DATA            3
#define STATE_TX_DATA_WAIT_ECHO  4
#define STATE_TX_BREAK           5

#define PRESCALE 8
#define BIT_TIME  ((uint8_t)((F_CPU / BAUD) / PRESCALE))
#define TX_TIMEOUT 10 /* x100ms = 1 second */

#define USE_USART 1
#include "core/usart.h"

static volatile uint8_t state = STATE_RX;
static volatile uint8_t bit_counter = 0;
static volatile uint8_t tx_timeout = 0;
static volatile uint8_t tx_packet_start;
static volatile uint8_t last_sent_byte;
static volatile uint8_t last_send_dest;

static volatile enum {
    NOT_WAITING,
    WAIT_FOR_ACK,
    WAIT_FOR_RESPONSE
} response_wait_mode = NOT_WAITING;

/* We generate our own usart init module, for our usart port */
generate_usart_init()

void
ems_uart_init(void)
{
  usart_init();

  PIN_CLEAR(EMS_UART_TX);
  DDR_CONFIG_OUT(EMS_UART_TX);

  TC2_PRESCALER_8;
  TC2_MODE_CTC;
}

void
ems_uart_got_response(void)
{
  if (response_wait_mode == WAIT_FOR_RESPONSE) {
    /* pretend being polled to trigger sending terminating address */
    ems_poll_address = OUR_EMS_ADDRESS | EMS_REQUEST_MASK;
    response_wait_mode = NOT_WAITING;
  }
}

static inline uint8_t
is_polled(void)
{
  uint8_t polled = (ems_poll_address == (OUR_EMS_ADDRESS | EMS_REQUEST_MASK));
  if (response_wait_mode == WAIT_FOR_ACK) {
    if (ems_poll_address == EMS_RESPONSE_OK) {
      UPDATE_STATS(onebyte_ack_packets, 1);
    }
    if (ems_poll_address == EMS_RESPONSE_FAIL) {
      UPDATE_STATS(onebyte_nack_packets, 1);
    }
    if (ems_poll_address == EMS_RESPONSE_OK
        || ems_poll_address == EMS_RESPONSE_FAIL) {
      ems_add_source_to_eop(last_send_dest);
      polled = 1;
    }
  }
  if (polled) {
    response_wait_mode = NOT_WAITING;
  }
  ems_poll_address = 0;
  return polled;
}

static void
switch_mode(uint8_t tx)
{
  uint8_t reg = usart(UCSR, B);
  if (tx) {
    reg &= ~(_BV(usart(RXEN)) | _BV(usart(RXCIE)));
    reg |= _BV(usart(UDRIE));
  } else {
    reg &= ~(_BV(usart(UDRIE)));
    reg |= _BV(usart(RXEN)) | _BV(usart(RXCIE));
  }
  usart(UCSR, B) = reg;
}

static void
go_to_rx(void)
{
  ems_set_led(LED_TX, 0, 0);
  /* drain input buffer */
  while (usart(UCSR,A) & _BV(usart(RXC))) {
    uint8_t data = usart(UDR);
    (void) data;
  }
  switch_mode(0);
  state = STATE_RX;
}

void
ems_uart_periodic(void)
{
  if (tx_timeout == 0) {
    return;
  }

  tx_timeout--;
  if (tx_timeout == 0) {
    go_to_rx();
  }
}

static void
start_break(void)
{
  usart(UCSR,B) &= ~(_BV(usart(UDRIE)) |
                     _BV(usart(TXEN))  |
                     _BV(usart(TXCIE)));
  bit_counter = 11;
  TC2_COUNTER_COMPARE = BIT_TIME;
  TC2_COUNTER_CURRENT = 0;
  TC2_INT_COMPARE_ON;
  state = STATE_TX_BREAK;
}

ISR(TC2_VECTOR_COMPARE)
{
  bit_counter--;
  if (bit_counter == 0) {
    TC2_INT_COMPARE_OFF;
    usart(UCSR,B) |= _BV(usart(TXEN));
    tx_timeout = 0;
    go_to_rx();
  }
}

ISR(usart(USART, _TX_vect))
{
  /* TX finished, now send break */
  start_break();
}

ISR(usart(USART,_UDRE_vect))
{
  switch (state) {
    case STATE_TX_ADDR:
      last_sent_byte = OUR_EMS_ADDRESS;
      usart(UDR) = OUR_EMS_ADDRESS;
      state = STATE_TX_ADDR_WAIT_ECHO;
      tx_timeout = TX_TIMEOUT;
      tx_packet_start = ems_send_buffer.sent;
      switch_mode(0);
      break;
    case STATE_TX_DATA:
      if (ems_send_buffer.sent < ems_send_buffer.len) {
        uint8_t byte = ems_send_buffer.data[ems_send_buffer.sent];
        if (ems_send_buffer.sent == tx_packet_start) {
          /* byte is the destination address */
          if (byte & EMS_REQUEST_MASK) {
            response_wait_mode = WAIT_FOR_RESPONSE;
          } else {
            response_wait_mode = WAIT_FOR_ACK;
          }
          last_send_dest = byte & ~EMS_REQUEST_MASK;
          ems_set_led(LED_TX, 1, 0);
        }
        ems_send_buffer.sent++;
        last_sent_byte = byte;
        usart(UDR) = byte;
        state = STATE_TX_DATA_WAIT_ECHO;
        tx_timeout = TX_TIMEOUT;
        switch_mode(0);
      } else {
        /* wait for TX to finish */
        usart(UCSR,B) &= ~(_BV(usart(UDRIE)));
        usart(UCSR,B) |= _BV(usart(TXCIE));
      }
      break;
    case STATE_TX_BREAK:
    default:
      /* Disable this interrupt */
      usart(UCSR,B) &= ~(_BV(usart(UDRIE)));
      go_to_rx();
      break;
  }
}

ISR(usart(USART,_RX_vect))
{
  uint8_t status, data;

  switch (state) {
    case STATE_TX_ADDR_WAIT_ECHO:
    case STATE_TX_DATA_WAIT_ECHO:
      status = usart(UCSR, A);

      if (status & _BV(usart(RXC))) {
        tx_timeout = 0;
        data = usart(UDR);
        if (last_sent_byte != data) {
          /* mismatch -> abort */
          EMSERRORDEBUG("Last sent byte %02x, echo %02x -> MISMATCH\n",
                        last_sent_byte, data);
          ems_send_buffer.sent = tx_packet_start;
          start_break();
        } else {
          state = STATE_TX_DATA;
        }
        switch_mode(1);
      }
      break;
    default:
      while ((status = usart(UCSR,A)) & _BV(usart(RXC))) {
        uint8_t real_status = 0;

        data = usart(UDR);

        if (status & _BV(usart(FE))) {
          real_status |= FRAMEEND;
        }
        if (status & (_BV(usart(DOR)) | _BV(usart(UPE)))) {
          real_status |= ERROR;
        }

        ems_uart_process_input_byte(data, real_status);
      }

      if (is_polled()) {
        UPDATE_STATS(onebyte_own_packets, 1);
        state = STATE_TX_ADDR;
        switch_mode(1);
#ifdef EMS_DEBUG
        if (ems_send_buffer.sent != ems_send_buffer.len) {
          EMSIODEBUG("Sending %d bytes\n",
                     ems_send_buffer.len - ems_send_buffer.sent);
        }
#endif
      }
      break;
  }
}

/*
  -- Ethersex META --
  timer(5, ems_uart_periodic())
*/
