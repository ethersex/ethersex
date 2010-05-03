/*
 * Copyright(C) 2010 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include "config.h"
#include <avr/io.h>

uint8_t 
spi_send(uint8_t outdata)
{
  DDR_CONFIG_IN(SOFT_SPI_MISO);

  uint8_t j, indata = indata;
  for(j = 0; j < 8; j++)
  {
    if(outdata & 0x80)
      PIN_SET(SOFT_SPI_MOSI);
    else
      PIN_CLEAR(SOFT_SPI_MOSI);

    PIN_SET(SOFT_SPI_SCK);
    indata <<= 1;

    if(PIN_HIGH(SOFT_SPI_MISO))
      indata |= 1;

    PIN_CLEAR(SOFT_SPI_SCK);

    outdata <<= 1;
  }

  DDR_CONFIG_OUT(SOFT_SPI_MISO);
  return indata;
}

