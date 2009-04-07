/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include "config.h"
#include "core/bit-macros.h"
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
static uint8_t recv_len, sent;
static int16_t write_len;
static volatile uint8_t must_parse;

void
ecmd_serial_usart_init(void) {
  recv_len = 0;
  must_parse = 0;
  write_len = 0;
  /* Initialize the usart module */
  usart_init();
#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
  DDR_CONFIG_OUT(ECMD_SERIAL_USART_TX);
  PIN_CLEAR(ECMD_SERIAL_USART_TX);
#endif
}

void
ecmd_serial_usart_periodic(void) 
{
  if (must_parse && !write_len) {
    /* we have a request */
    if (recv_len <= 1) return;
    write_len = ecmd_parse_command(recv_buffer, write_buffer, sizeof(write_buffer));
    must_parse = 0;
    if (write_len < -10) {
      write_len = -( 10 + write_len);
      must_parse = 1;
    } else if (write_len < 0)
      return;
    else {
      recv_len = 0;
    }

    write_buffer[write_len++] = '\r';
    write_buffer[write_len++] = '\n';
    
#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
    PIN_SET(ECMD_SERIAL_USART_TX);
#endif

    /* Enable the tx interrupt and send the first character */
    sent = 1;
    usart(UDR) = write_buffer[0];
    usart(UCSR,B) |= _BV(usart(TXCIE));

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
  if (must_parse) return;


  if (data == '\n' || data == '\r' || recv_len == sizeof(recv_buffer)) {
    recv_buffer[recv_len] = 0;
    must_parse = 1;
    usart(UDR) = '\r';
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = '\n';
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    return ;
  }

  usart(UDR) = data;
  recv_buffer[recv_len++] = data;
}

SIGNAL(usart(USART,_TX_vect))
{
  if (sent < write_len) {
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = write_buffer[sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));

    write_len = 0;

#ifdef ECMD_SERIAL_USART_RS485_SUPPORT
    PIN_CLEAR(ECMD_SERIAL_USART_TX);
#endif
  }
}
#endif
