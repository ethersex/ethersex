/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2012 Erik Kunze <ethersex@erik-kunze.de>
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

#if defined(RFM12_SUPPORT) || defined(ENC28J60_SUPPORT) || \
    defined(DATAFLASH_SUPPORT) || defined(SD_READER_SUPPORT)

void
spi_init(void)
{
  /* Input and Output configuration is done in the beginning of main(), so it
   * doesn't have to be done here
   */

  /* Set the chip-selects as high */

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

#if defined(SPI_CS_SD_READER_PIN) && defined(SD_NETIO_ADDON_WORKAROUND)
  PIN_SET(SPI_CS_SD_READER);
#endif

#ifndef SOFT_SPI_SUPPORT
  /* enable spi, set master and clock modes (f/2) */
  _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
  _SPSR0 = _BV(_SPI2X0);
#endif
}


#ifndef SOFT_SPI_SUPPORT
static void
spi_wait_busy(void)
{
#   ifdef SPI_TIMEOUT
  uint8_t timeout = 200;

  while (!(_SPSR0 & _BV(_SPIF0)) && timeout > 0)
    timeout--;

  if (timeout == 0)
    debug_printf("ERROR: spi timeout reached!\r\n");
#   else
  while (!(_SPSR0 & _BV(_SPIF0)));
#   endif

}

uint8_t noinline
spi_send(uint8_t data)
{
  _SPDR0 = data;
  spi_wait_busy();

  return _SPDR0;
}
#endif /* !SOFT_SPI_SUPPORT */

#endif /* DATAFLASH_SUPPORT || ENC28J60_SUPPORT || RFM12_SUPPORT ||
          SD_READER_SUPPORT */
