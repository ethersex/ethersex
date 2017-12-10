/*
 * Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2017 Erik Kunze <ethersex@erik-kunze.de>
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

#include <avr/io.h>
#include "core/spi.h"
#include "spi.h"
#include "debug.h"
#include "config.h"
#include "hardware/radio/rfm12/rfm12.h"

#define USE_USART SPI_USE_USART
#define BAUD 9600               /* Dummy */
#include "core/usart.h"

#if defined(RFM12_SUPPORT)          || \
    defined(ENC28J60_SUPPORT)       || \
    defined(DATAFLASH_SUPPORT)      || \
    defined(SD_READER_SUPPORT)      || \
    defined(USTREAM_SUPPORT)        || \
    defined(SER_RAM_23K256_SUPPORT) || \
    defined(S1D15G10_SUPPORT)       || \
    defined(GLCD_SPI_SUPPORT)

void
spi_init(void)
{
  usart(UBRR) = 0;

  /* Set the chipselects as high */
  /* We need the DDR configuration here, because this enables master mode of
   * usart_spi
   */

#ifdef ENC28J60_SUPPORT
  PIN_SET(SPI_CS_NET);
#endif

#ifdef SER_RAM_23K256_SUPPORT
  PIN_SET(SPI_CS_23K256);
#endif

#ifdef RFM12_SUPPORT
  for (int8_t modul = 0; modul < RFM12_MODULE_COUNT; modul++)
  {
    *rfm12_moduls[modul].rfm12_port |= rfm12_moduls[modul].rfm12_mask;
  }
#endif

#ifdef DATAFLASH_SUPPORT
  PIN_SET(SPI_CS_DF);
#endif

#ifdef USTREAM_SUPPORT
  PIN_SET(VS1053_CS);
#endif

#if defined(SPI_CS_SD_READER_PIN)
  PIN_SET(SPI_CS_SD_READER);
#endif

#ifdef S1D15G10_SUPPORT
  PIN_SET(S1D15G10_CS);
#endif

#ifdef GLCD_SPI_SUPPORT
  PIN_SET(GLCD_CS);
#endif

  /* Set MSPI mode of operation and SPI data mode 0. */
  usart(UCSR, C) = _BV(usart(UMSEL, 1)) | _BV(usart(UMSEL, 0))
    | _BV(usart(UCPHA)) | _BV(usart(UCPOL));
  /* Enable receiver and transmitter. */
  usart(UCSR, B) = _BV(usart(RXEN)) | _BV(usart(TXEN));
  /* Set baud rate. */
  /* IMPORTANT: The Baud Rate must be set after the transmitter is enabled
   * */
  /* Set to the higest available Baudrate: fosc/2 */
  usart(UBRR) = 0;
}

static void
spi_wait_busy(void)
{
#ifdef SPI_TIMEOUT
  uint8_t timeout = 200;

  while (!(usart(UCSR, A) & _BV(usart(RXC))) && timeout > 0)
    timeout--;

  if (timeout == 0)
    debug_printf("ERROR: spi timeout reached!\r\n");
#else
  while (!(usart(UCSR, A) & _BV(usart(RXC))));
#endif
}

uint8_t noinline
spi_send(uint8_t data)
{
  usart(UDR) = data;
  spi_wait_busy();

  return usart(UDR);
}

#endif
