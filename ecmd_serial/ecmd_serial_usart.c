/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../config.h"
#include "../bit-macros.h"
#include "ecmd_serial_usart.h"
#include "../ecmd_parser/ecmd.h"

#define USE_USART ECMD_SERIAL_USART_USE_USART
#define BAUD ECMD_SERIAL_BAUDRATE
#include "../usart.h"



#ifdef ECMD_SERIAL_USART_SUPPORT

/* We generate our own usart init module, for our usart port */
generate_usart_init()

static char recv_buffer[ECMD_SERIAL_USART_BUFFER_LEN];
static char write_buffer[ECMD_SERIAL_USART_BUFFER_LEN + 2];
static uint8_t recv_len, write_len, sent;

void ecmd_serial_usart_init(void) {
  recv_len = 0;
  /* Initialize the usart module */
  usart_init();
  DDRA = 0xC0;
#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
  DDR_CONFIG_OUT(ECMD_SERIAL_USART_TX);
  PIN_CLEAR(ECMD_SERIAL_USART_TX);
#endif
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
  if (data == '\r') return; /* We ignore '\r' */

  recv_buffer[recv_len++] = data;

  if (data == '\n' || recv_len == sizeof(recv_buffer)) {
    PORTA ^= 0x80;
    /* we have a request */
    recv_buffer[recv_len - 1] = 0;
    if (recv_len == 1) return;
    write_len = ecmd_parse_command(recv_buffer, write_buffer, sizeof(write_buffer));
    write_buffer[write_len++] = '\r';
    write_buffer[write_len++] = '\n';
    
#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
    PIN_SET(ECMD_SERIAL_USART_TX);
#endif

    /* Enable the tx interrupt and send the first character */
    sent = 1;
    usart(UCSR,B) |= _BV(usart(TXCIE));
    usart(UDR) = write_buffer[0];

    recv_len = 0;
  }
}

SIGNAL(usart(USART,_TX_vect))
{
  if (sent < write_len) {
    usart(UDR) = write_buffer[sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));

#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
    PIN_CLEAR(ECMD_SERIAL_USART_TX);
#endif
  }
}
#endif
