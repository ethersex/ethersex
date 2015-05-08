/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include "config.h"
#include "bsbport.h"
#include "bsbport_net.h"
#include "bsbport_helper.h"
#include "bsbport_rx.h"

#define USE_USART BSBPORT_USE_USART
#define BAUD BSBPORT_BAUDRATE
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_bsbport_usart_init_8O1();

struct bsbport_buffer_net bsbport_recvnet_buffer;
struct bsbport_buffer_net bsbport_send_buffer;
struct bsbport_buffer_rx bsbport_recv_buffer;
uint16_t bsbport_rx_ok;
uint16_t bsbport_rx_crcerror;
uint16_t bsbport_rx_lenghtunder;
uint16_t bsbport_rx_lenghtover;
uint16_t bsbport_rx_frameerror;
uint16_t bsbport_rx_overflow;
uint16_t bsbport_rx_parityerror;
uint16_t bsbport_rx_bufferfull;
uint16_t bsbport_rx_dropped;
uint16_t bsbport_rx_net_bufferfull;
uint16_t bsbport_eth_retransmit;

#if BSBPORT_FLUSH > 0
uint8_t bsbport_lf;
#endif

void
bsbport_init(void)
{
  usart_init();
}

uint8_t
bsbport_txstart(const uint8_t * const data, const uint16_t len)
{
  const uint16_t diff = bsbport_send_buffer.len - bsbport_send_buffer.sent;
  if (diff == 0)
  {
    /* Copy the data to the send buffer */
    memcpy(bsbport_send_buffer.data, data, len);
    bsbport_send_buffer.len = len;
    goto start_sending;
    /* The actual packet can be pushed into the buffer */
  }
  else if ((diff + len) < BSBPORT_BUFFER_LEN)
  {
    memmove(bsbport_send_buffer.data,
            bsbport_send_buffer.data + bsbport_send_buffer.sent, diff);
    memcpy(bsbport_send_buffer.data + diff, data, len);
    bsbport_send_buffer.len = diff + len;
    goto start_sending;
  }
  return 0;
start_sending:
  bsbport_send_buffer.sent = 1;
  /* Enable the tx interrupt and send the first character */
  usart(UCSR, B) |= _BV(usart(TXCIE));
  usart(UDR) = bsbport_send_buffer.data[0] ^ 0xFF;
  return 1;
}

ISR(usart(USART, _TX_vect))
{
  if (bsbport_send_buffer.sent < bsbport_send_buffer.len)       /*      Send Bytes in Buffer     */
  {
    usart(UDR) = bsbport_send_buffer.data[bsbport_send_buffer.sent++] ^ 0xFF;
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
      uint8_t v = usart(UDR);
      (void) v;
    }
    else
    {
      uint8_t v = usart(UDR);
      if (bsbport_recvnet_buffer.len < BSBPORT_BUFFER_LEN)
        bsbport_recvnet_buffer.data[bsbport_recvnet_buffer.len++] = v ^ 0xFF;
      else
        bsbport_rx_net_bufferfull++;
      if (bsbport_recv_buffer.len < BSBPORT_BUFFER_LEN)
        bsbport_recv_buffer.data[bsbport_recv_buffer.len++] = v ^ 0xFF;
      else
        bsbport_rx_bufferfull++;
#if BSBPORT_FLUSH > 0
      if ((v ^ 0xFF) == SOT_BYTE)
        bsbport_lf = 1;
#endif
    }
  }
}

/*
  -- Ethersex META --
  dnl bsbport_init call must be done after network_init (according to earlier
  dnl comments.  Therefore we initialize via net_init and control the
  dnl order via the Makefile.
  
  header(protocols/bsbport/bsbport.h)

  net_init(bsbport_init)
*/
