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
#include <stdio.h>
#include <stdint.h>
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../config.h"
#include "../net/mcuf_net.h"
#include "../syslog/syslog.h"
#include "mcuf.h"
#include "../uip/uip.h"

#include "../pinning.c"

#ifdef MCUF_SUPPORT

#define USE_USART MCUF_USE_USART
#include "../usart.h"

struct {
  uint8_t len;
  uint8_t sent;
  uint8_t data[170];
} buffer;

struct mcuf_packet {
  uint16_t magic[2];
  uint16_t height;
  uint16_t width;
  uint16_t channels;
  uint16_t maxval;
  uint8_t data[];
};

void mcuf_senddata(uint16_t height, uint16_t width, uint16_t channels, uint16_t maxval);
void tx_start(uint8_t len);


/* We generate our own usart init module, for our usart port */
generate_usart_init(MCUF_UART_UBRR)


void
mcuf_init(void)
{
    /* Initialize the usart module */
    usart_init();
    /* Disable the receiver */
    usart(UCSR,B) &= ~_BV(usart(RXCIE));

    buffer.len = 0;
    buffer.sent = 0;
}

void
mcuf_newdata(void) 
{
  /* If we send a packet, drop the new packet */
  if (buffer.sent < buffer.len) return;

  /* MCUF Magic bytes */
  if ( strncmp(uip_appdata, "\x23\x54\x26\x66", 4) == 0) {
    struct mcuf_packet *pkt = (struct mcuf_packet *)uip_appdata;
    memcpy(buffer.data, pkt->data, 144);
    memset(buffer.data, 0, 156);
    buffer.data[13] = 255;
    buffer.data[28] = 255;
    mcuf_senddata(ntohs(pkt->height), ntohs(pkt->width), 1, 255); 
  }
}

void
mcuf_senddata(uint16_t height, uint16_t width, uint16_t channels, uint16_t maxval)
{
  syslog_sendf("%x %x %x", height, width, channels);
  memcpy_P(buffer.data, PSTR("\x23\x54\x26\x66\x00\x08\x00\x12\x00\x01\x00\xff"), 12);
#if 0
  ((uint16_t *) &buffer.data[4])[0] = ntohs(height);
  ((uint16_t *) &buffer.data[4])[1] = ntohs(width);
  ((uint16_t *) &buffer.data[4])[2] = ntohs(channels);
  ((uint16_t *) &buffer.data[4])[3] = ntohs(maxval);
#endif
  

  /* send 144 bytes od data */
  tx_start(156);
}

void
tx_start(uint8_t len)
{
    buffer.len = len;
    buffer.sent = 1;
    /* Enable the tx interrupt and send the first character */
    usart(UCSR,B) |= _BV(usart(TXCIE));
    usart(UDR) = buffer.data[0];
}

SIGNAL(usart(USART,_TX_vect))
{
  if (buffer.sent < buffer.len) {
    usart(UDR) = buffer.data[buffer.sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
  }
}
#endif
