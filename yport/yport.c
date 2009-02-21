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
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../config.h"
#include "yport.h"
#include "yport_net.h"

#ifdef YPORT_SUPPORT

#define USE_USART YPORT_USE_USART 
#define BAUD YPORT_BAUDRATE
#include "../usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

struct yport_buffer yport_send_buffer;
struct yport_buffer yport_recv_buffer;

void
yport_init(void) 
{
  usart_init();

#if 0 //ndef TEENSY_SUPPORT
    uint16_t ubrr = usart_baudrate(eeprom_read_word(&(((struct eeprom_config_ext_t *)
                                     EEPROM_CONFIG_EXT)->usart_baudrate)));
    usart(UBRR,H) = HI8(ubrr);
    usart(UBRR,L) = LO8(ubrr);
#endif
}

uint8_t
yport_rxstart(uint8_t *data, uint8_t len) 
{
  uint8_t diff = yport_send_buffer.len - yport_send_buffer.sent;
  if (diff == 0) {
    /* Copy the data to the send buffer */
    memcpy(yport_send_buffer.data, data, len);
    yport_send_buffer.len = len;
    goto start_sending;
  /* The actual packet can be pushed into the buffer */
  } else if (((uint16_t) (diff + len)) < YPORT_BUFFER_LEN) {
    memmove(yport_send_buffer.data, yport_send_buffer.data + yport_send_buffer.sent, diff);
    memcpy(yport_send_buffer.data + diff, data, len);
    yport_send_buffer.len = diff + len;
    goto start_sending;
  }
  return 0;
start_sending:
    yport_send_buffer.sent = 1;
    /* Enable the tx interrupt and send the first character */
    usart(UCSR,B) |= _BV(usart(TXCIE));
    usart(UDR) = yport_send_buffer.data[0];
    return 1;
}


SIGNAL(usart(USART,_TX_vect))
{
  if (yport_send_buffer.sent < yport_send_buffer.len) {
    usart(UDR) = yport_send_buffer.data[yport_send_buffer.sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
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
  if (yport_recv_buffer.len < YPORT_BUFFER_LEN)
    yport_recv_buffer.data[yport_recv_buffer.len++] = data;
}
#endif
