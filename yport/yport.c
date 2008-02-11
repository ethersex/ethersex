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
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "../bit-macros.h"
#include "../config.h"
#include "yport.h"
#include "../net/yport_net.h"

#ifdef YPORT_SUPPORT

struct yport_buffer yport_send_buffer;
struct yport_buffer yport_recv_buffer;

void
yport_init(void) 
{
    /* The ATmega644 datasheet suggests to clear the global
       interrupt flags on initialization ... */
    uint8_t sreg = SREG; cli();

    /* set baud rate */
    _UBRRH_UART0 = HI8(YPORT_UART_UBRR);
    _UBRRL_UART0 = LO8(YPORT_UART_UBRR);

#ifdef URSEL
    /* set mode: 8 bits, 1 stop, no parity, asynchronous usart
       and Set URSEL so we write UCSRC and not UBRRH */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01) | _BV(URSEL);
#else
    /* set mode: 8 bits, 1 stop, no parity, asynchronous usart */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);
#endif

    /* Enable the RX interrupt and receiver and transmitter */
    _UCSRB_UART0 |= _BV(_TXEN_UART0) | _BV(_RXEN_UART0) | _BV(_RXCIE_UART0);

    /* Go! */
    SREG = sreg;
}

void
yport_rxstart(uint8_t *data, uint8_t len) 
{
  if (yport_send_buffer.len == yport_send_buffer.sent) {
    /* Copy the data to the send buffer */
    memcpy(yport_send_buffer.data, data, YPORT_BUFFER_LEN);
    if (len < YPORT_BUFFER_LEN)
      yport_send_buffer.len = len;
    else
      yport_send_buffer.len = YPORT_BUFFER_LEN;
    yport_send_buffer.sent = 1;
    /* Enable the tx interrupt and send the first character */
    _UCSRB_UART0 |= _BV(_TXCIE_UART0);
    _UDR_UART0 = yport_send_buffer.data[0];
  }
}


SIGNAL(USART0_TX_vect)
{
  if (yport_send_buffer.sent < yport_send_buffer.len) {
    _UDR_UART0 = yport_send_buffer.data[yport_send_buffer.sent++];
  } else {
    /* Disable this interrupt */
    _UCSRB_UART0 &= ~(_BV(_TXCIE_UART0));
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
  if (yport_recv_buffer.len < YPORT_BUFFER_LEN)
    yport_recv_buffer.data[yport_recv_buffer.len++] = data;
}
#endif
