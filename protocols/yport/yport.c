/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "core/eeprom.h"

#define YPORT_USART_CONFIG_8N1 1
#define YPORT_USART_CONFIG_8E2 2

#include "config.h"
#include "yport.h"
#include "yport_net.h"


#define USE_USART YPORT_USE_USART
#define BAUD YPORT_BAUDRATE
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
#if YPORT_USART_CONFIG == YPORT_USART_CONFIG_8N1
	generate_usart_init()
#elif YPORT_USART_CONFIG == YPORT_USART_CONFIG_8E2
	generate_usart_init_8E2()
#else
	#error "YPORT_USART_CONFIG not correctly defined"
#endif

struct yport_buffer yport_send_buffer;
struct yport_buffer yport_recv_buffer;
#ifdef DEBUG_YPORT
uint16_t yport_rx_frameerror;
uint16_t yport_rx_overflow;
uint16_t yport_rx_parityerror;
uint16_t yport_rx_bufferfull;
uint16_t yport_eth_retransmit;
#endif
#if YPORT_FLUSH > 0
uint8_t yport_lf;
#endif

void yport_init(void)
{
  usart_init();
}

uint8_t
yport_rxstart(uint8_t * data, uint16_t len)
{
  uint16_t diff = yport_send_buffer.len - yport_send_buffer.sent;
  if (diff == 0)
  {
    /* Copy the data to the send buffer */
    memcpy(yport_send_buffer.data, data, len);
    yport_send_buffer.len = len;
    goto start_sending;
    /* The actual packet can be pushed into the buffer */
  }
  else if ((diff + len) < YPORT_BUFFER_LEN)
  {
    memmove(yport_send_buffer.data,
            yport_send_buffer.data + yport_send_buffer.sent, diff);
    memcpy(yport_send_buffer.data + diff, data, len);
    yport_send_buffer.len = diff + len;
    goto start_sending;
  }
  return 0;
start_sending:
  yport_send_buffer.sent = 1;
  /* Enable the tx interrupt and send the first character */
  usart(UCSR, B) |= _BV(usart(TXCIE));
  usart(UDR) = yport_send_buffer.data[0];
  return 1;
}


ISR(usart(USART, _TX_vect))
{
  if (yport_send_buffer.sent < yport_send_buffer.len)
  {
    usart(UDR) = yport_send_buffer.data[yport_send_buffer.sent++];
  }
  else
  {
    /* Disable this interrupt */
    usart(UCSR, B) &= ~(_BV(usart(TXCIE)));
  }
}

ISR(usart(USART, _RX_vect))
{
  while (usart(UCSR, A) & _BV(usart(RXC)))
  {
    if (usart(UCSR, A) & (_BV(usart(FE)) | _BV(usart(DOR)) | _BV(usart(UPE))))
    {
#ifdef DEBUG_YPORT
      if (usart(UCSR, A) & _BV(usart(FE)))
        yport_rx_frameerror++;
      if (usart(UCSR, A) & _BV(usart(DOR)))
        yport_rx_overflow++;
      if (usart(UCSR, A) & _BV(usart(UPE)))
        yport_rx_parityerror++;
#endif
      uint8_t v = usart(UDR);
      (void) v;
    }
    else
    {
      uint8_t v = usart(UDR);
      if (yport_recv_buffer.len < YPORT_BUFFER_LEN)
        yport_recv_buffer.data[yport_recv_buffer.len++] = v;
#ifdef DEBUG_YPORT
      else
        yport_rx_bufferfull++;
#endif
#if YPORT_FLUSH > 0
      if (v == 0x0A)
        yport_lf = 1;
#endif
    }
  }
}

/*
  -- Ethersex META --
  dnl yport_init call must be done after network_init (according to earlier
  dnl comments.  Therefore we initialize via net_init and control the
  dnl order via the Makefile.
  
  header(protocols/yport/yport.h)

  net_init(yport_init)
*/
