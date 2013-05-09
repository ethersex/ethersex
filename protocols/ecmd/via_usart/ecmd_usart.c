/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3) as
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
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "ecmd_usart.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#define USE_USART ECMD_SERIAL_USART_USE_USART
#define BAUD ECMD_SERIAL_BAUDRATE
#include "core/usart.h"
#include "pinning.c"


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

  RS485_TE_SETUP;             // configure RS485 transmit enable as output
  RS485_DISABLE_TX;           // disable RS485 transmitter
  usart_init();               // initialize the usart module
}

void
ecmd_serial_usart_periodic(void)
{
  if (must_parse && write_len == 0) {
    /* we have a request */
    must_parse = 0;

    if (recv_len <= 1) {
      recv_len = 0;
      return;
    }

    write_len = ecmd_parse_command(recv_buffer, write_buffer, sizeof(write_buffer));
    if (is_ECMD_AGAIN(write_len)) {
      /* convert ECMD_AGAIN back to ECMD_FINAL */
      write_len = ECMD_AGAIN(write_len);
      must_parse = 1;
    }
    else if (is_ECMD_ERR(write_len))
      return;
    else {
      recv_len = 0;
    }

    write_buffer[write_len++] = '\r';
    write_buffer[write_len++] = '\n';

    RS485_ENABLE_TX;

    /* Enable the tx interrupt and send the first character */
    sent = 1;
    usart(UDR) = write_buffer[0];
    usart(UCSR,B) |= _BV(usart(TXCIE));

  }
}

ISR(usart(USART,_RX_vect))
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

#ifndef ECMD_SERIAL_USART_NO_ECHO
  usart(UDR) = data;
#endif /* ECMD_SERIAL_USART_NO_ECHO */

  recv_buffer[recv_len++] = data;
}

ISR(usart(USART,_TX_vect))
{
  if (sent < write_len) {
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = write_buffer[sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));

    write_len = 0;

    RS485_DISABLE_TX;
  }
}

/*
  -- Ethersex META --
  header(protocols/ecmd/via_usart/ecmd_usart.h)
  init(ecmd_serial_usart_init)
  timer(1,ecmd_serial_usart_periodic())
*/
