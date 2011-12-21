/*
 * Copyright (c) 2011 by Daniel Walter <fordprfkt@googlemail.com>
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


#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "core/spi.h"
#include "sram_23k256.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef SER_RAM_23K256_SUPPORT

#define cs_low()  PIN_CLEAR(SPI_CS_HARDWARE)
#define cs_high() PIN_SET(SPI_CS_HARDWARE)

#define SIZE 32768

int16_t sram23k256_read(uint16_t address_ui16, uint8_t *dataPtr_pui8, uint8_t len_ui8)
{
  uint16_t ctr = 0;

  SERRAMDEBUG ("read\n");

  /* aquire device */
  cs_low();

  /* send command */
  spi_send(SRAM_23K256_READ);
  spi_send(address_ui16);
  for (ctr = 0; ctr < len_ui8; ctr++)
  {
    dataPtr_pui8[ctr] = spi_send(0);
  }

  /* release device */
  cs_high();

  return 0;
}

int16_t sram23k256_write(uint16_t address_ui16, uint8_t *dataPtr_pui8, uint8_t len_ui8)
{
  uint16_t ctr = 0;

  SERRAMDEBUG ("write\n");

  /* aquire device */
  cs_low();

  /* send command */
  spi_send(SRAM_23K256_WRITE);
  spi_send(address_ui16);
  for (ctr = 0; ctr < len_ui8; ctr++)
  {
     spi_send(dataPtr_pui8[ctr]);
  }

  /* release device */
  cs_high();

  return 0;
}

/*
  If enabled in menuconfig, this function is called during boot up of ethersex
*/
int16_t sram23k256_init(void)
{
  uint8_t data = 0;
  uint16_t ctr = 0;
  bool fail = false;

  PIN_SET(SRAM_23K256_HOLD);

  SERRAMDEBUG ("init\n");

  /* aquire device */
  cs_low();

  /* send command */
  spi_send(SRAM_23K256_WRSR);
  spi_send(SRAM_23K256_HOLD|SRAM_23K256_MODE_SEQ);

  cs_high();

#ifdef SER_RAM_23K256_RAMTEST

  cs_low();
  spi_send(SRAM_23K256_WRITE);
  spi_send(0);
  spi_send(0);

  for (ctr = 0; ctr < SIZE; ctr++)
  {
    spi_send(data);
    data++;
  }
  cs_high();

  cs_low();
  spi_send(SRAM_23K256_READ);
  spi_send(0);
  spi_send(0);

  data = 0;
  for (ctr = 0; ctr < SIZE; ctr++)
  {
    if (data != spi_send(0))
    {
      fail = true;
    }
    data++;
  }
  cs_high();

  if (true == fail)
  {
    SERRAMDEBUG ("RAM test failed!\n");
  }
  else
  {
    SERRAMDEBUG ("RAM test OK!\n");
  }
#endif

  cs_low();
  spi_send(SRAM_23K256_WRITE);
  spi_send(0);
  spi_send(0);

  for (ctr = 0; ctr < SIZE; ctr++)
  {
    spi_send(0);
  }

  /* release device */
  cs_high();

  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(hardware/serial_ram/23k256/sram_23k256.h)
  init(sram23k256_init)
*/
#endif /* SER_RAM_23K256_SUPPORT */
