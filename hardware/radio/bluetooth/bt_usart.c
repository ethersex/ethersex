/*
 * Copyright (c) 2013-2017 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <stdint.h>

#include "config.h"
#include "core/debug.h"

#include "bt.h"
#include "bt_usart.h"
#include "protocols/ecmd/via_bluetooth/ecmd_bt.h"


#ifdef ECMD_BLUETOOTH_SUPPORT
bool bt_init_finished;
#endif
#ifdef BLUETOOTH_ECMD
uint16_t bt_rx_frameerror;
uint16_t bt_rx_overflow;
uint16_t bt_rx_parityerror;
uint16_t bt_rx_bufferfull;
uint16_t bt_rx_count;
uint16_t bt_tx_count;
#endif


/* USART and FIFO code by Peter Daannegger
 * http://www.mikrocontroller.net/topic/101472 */
#define BLUETOOTH_UART_RX_SIZE  16
#define BLUETOOTH_UART_TX_SIZE  8

static uint8_t rx_buff[BLUETOOTH_UART_RX_SIZE];
static uint8_t rx_in;
static uint8_t rx_out;
static uint8_t tx_buff[BLUETOOTH_UART_TX_SIZE];
static uint8_t tx_in;
static uint8_t tx_out;

#define NELEMS(x)               (sizeof(x)/sizeof(x[0]))
#define ROLLOVER(x, max)        x = ++x >= max ? 0 : x
                                /* count up and wrap around */

#define USE_USART BLUETOOTH_USE_USART
#include "core/usart.h"


void
bt_usart_init(const uint8_t hvalue, const uint8_t lvalue, const bool use2x)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    usart(UBRR, H) = hvalue;
    usart(UBRR, L) = lvalue;

    /* set mode 8N1: 8 bits, 1 stop, no parity, asynchronous usart
     * and set URSEL, if present, */
    usart(UCSR, C) = _BV(usart(UCSZ, 0)) | _BV(usart(UCSZ, 1)) | _BV_URSEL;
    /* Enable the RX interrupt and receiver and transmitter */
    usart(UCSR, B) |= _BV(usart(TXEN)) | _BV(usart(RXEN)) | _BV(usart(RXCIE));

    if (use2x)
      usart(UCSR, A) |= _BV(usart(U2X));
    else
      usart(UCSR, A) &= ~_BV(usart(U2X));
  }
}


ISR(usart(USART, _UDRE_vect))
{
#ifdef ECMD_BLUETOOTH_SUPPORT
  if (bt_init_finished)
    ecmd_bt_send();
#endif
  if (tx_in == tx_out)
  {                             /* nothing to sent */
    usart(UCSR, B) &= ~_BV(usart(UDRIE));       /* disable TX interrupt */
    return;
  }
  usart(UDR) = tx_buff[tx_out];
  ROLLOVER(tx_out, NELEMS(tx_buff));
#ifdef BLUETOOTH_ECMD
  bt_tx_count++;
#endif
}


ISR(usart(USART, _RX_vect))
{
  /* Ignore errors */
  uint8_t flags = usart(UCSR, A);
  if (flags & (_BV(usart(FE)) | _BV(usart(DOR)) | _BV(usart(UPE))))
  {
#ifdef BLUETOOTH_ECMD
    if (flags & _BV(usart(FE)))
      bt_rx_frameerror++;
    if (flags & _BV(usart(DOR)))
      bt_rx_overflow++;
    if (flags & _BV(usart(UPE)))
      bt_rx_parityerror++;
#endif
    flags = usart(UDR);         /* dummy read */
    return;
  }

  uint8_t i = rx_in;
  ROLLOVER(i, NELEMS(rx_buff));
  if (i == rx_out)
  {                             /* buffer overflow */
    usart(UCSR, B) &= ~_BV(usart(RXCIE));       /* disable RX interrupt */
    return;
  }

  rx_buff[rx_in] = usart(UDR);
  rx_in = i;
#ifdef BLUETOOTH_ECMD
  bt_rx_count++;
#endif
#ifdef ECMD_BLUETOOTH_SUPPORT
  if (bt_init_finished)
    ecmd_bt_recv();
#endif
}


uint8_t
bt_usart_free(void)
{
  uint8_t i = tx_in;
  ROLLOVER(i, NELEMS(tx_buff));
  return tx_out ^ i;
}


void
bt_usart_putchar(const uint8_t c)
{
  uint8_t i = tx_in;
  ROLLOVER(i, NELEMS(tx_buff));
  tx_buff[tx_in] = c;
  /* until at least one byte free */
  while (i == *(volatile uint8_t *) &tx_out);   /* tx_out modified by interrupt! */
  tx_in = i;
  usart(UCSR, B) |= _BV(usart(UDRIE));  /* enable TX interrupt */
}


uint8_t
bt_usart_avail(void)
{
  /* rx_in modified by interrupt! */
  return rx_out ^ *(volatile uint8_t *) &rx_in;
}


uint8_t
bt_usart_getchar(void)
{
  while (!bt_usart_avail());    /* until at least one byte in */
  uint8_t data = rx_buff[rx_out];
  ROLLOVER(rx_out, NELEMS(rx_buff));
  usart(UCSR, B) |= _BV(usart(RXCIE));  /* enable RX interrupt */
  return data;
}
