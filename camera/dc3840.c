/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "../config.h"

#ifdef DC3840_SUPPORT

#include <util/delay.h>
#include "dc3840.h"

/* USART cruft. */
#define USE_USART DC3840_USE_USART
#define BAUD 921600
#include "../usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

static uint8_t dc3840_syncing;
static volatile uint16_t dc3840_sync_rx_len;

static void dc3840_send_uart (uint8_t byte) __attribute__ ((noinline));
static void dc3840_send_command (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t)
     __attribute__ ((noinline));


static void
dc3840_send_uart (uint8_t byte)
{
  while (!(usart(UCSR,A) & _BV(usart(UDRE))));
  usart(UDR) = byte;
}


static void
dc3840_send_command (uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
{
  /* Send command sequence first */
  dc3840_send_uart (0xFF);
  dc3840_send_uart (0xFF);
  dc3840_send_uart (0xFF);

  /* Send payload. */
  dc3840_send_uart (a);
  dc3840_send_uart (b);
  dc3840_send_uart (c);
  dc3840_send_uart (d);
  dc3840_send_uart (e);
}

#include <avr/wdt.h>
void
dc3840_init (void)
{
  usart_init ();

  uint8_t sync_retries = 100;
  dc3840_syncing = 1;
  do
    {
      /* Send SYNC sequence. */
      dc3840_sync_rx_len = 0;
      dc3840_send_command (DC3840_CMD_SYNC, 0, 0, 0, 0);

      _delay_ms (1);
      wdt_kick ();
      if (dc3840_sync_rx_len >= 16) break;
    }
  while (-- sync_retries);

  if (dc3840_sync_rx_len < 16)
    {
      DC3840_DEBUG ("Failed to sync to camera (rx_len = %d).\n",
		    dc3840_sync_rx_len);
      return;
    }

  if (dc3840_sync_rx_len > 16)
    DC3840_DEBUG ("Received %d bytes from camera on SYNC.\n",
		  dc3840_sync_rx_len);

  /* We have received 16 bytes,
     -> ACK for our SYNC
     -> SYNC command from camera.

     We need to ACK the SYNC first. */
  dc3840_send_command (DC3840_CMD_ACK, DC3840_CMD_SYNC, 0, 0, 0);
  DC3840_DEBUG ("Successfully sync'ed to camera!\n");


}


SIGNAL(usart(USART,_RX_vect))
{
  uint8_t temp = usart (UDR);
  (void) temp;

  if (dc3840_syncing)
    dc3840_sync_rx_len ++;

}

#endif	/* DC3840_SUPPORT */
