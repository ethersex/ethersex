/*
*
* Copyright (c) 2012 by Daniel Walter <fordprfkt@googlemail.com>
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
#include "core/bit-macros.h"
#include "sram_23k256.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef SER_RAM_23K256_SUPPORT

/* Commands */
#define SRAM_23K256_WRSR  0x01 /* Write status register command */
#define SRAM_23K256_WRITE 0x02 /* Write memory command */
#define SRAM_23K256_READ  0x03 /* Read memory command */
#define SRAM_23K256_RDSR  0x05 /* Read status register command */

/* Access mode flags for status register*/
#define SRAM_23K256_MODE_BYTE 0   /* Byte-wise access */
#define SRAM_23K256_MODE_SEQ 64   /* Sequential access */
#define SRAM_23K256_MODE_PAGE 128 /* Page-wise access */

/* Hold flag for status register*/
#define SRAM_23K256_HOLD 1  /* Use HOLD pin */

/**
* @brief Read data from the serial RAM
*
* Adresses the serial RAM chip and reads the given number of bytes
* from the RAM, starting at address_ui16 into dataPtr_pui8.
*
* @param address_ui16 RAM address to start reading from.
* @param dataPtr_pui8 Pointer to destination
* @param len_ui8 Number of bytes to be read
*/
void sram23k256_read(uint16_t address_ui16, uint8_t dataPtr_pui8[], uint8_t len_ui8)
{
  uint16_t ctr = 0;

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* send command & address */
  spi_send(SRAM_23K256_READ);
  spi_send(HI8(address_ui16));
  spi_send(LO8(address_ui16));

  /* Read data from chip */
  for (ctr = 0; ctr < len_ui8; ctr++)
  {
    dataPtr_pui8[ctr] = spi_send(0);
  }

  /* Release device */
  PIN_SET(SPI_CS_23K256);
}

/**
* @brief Write data into the serial RAM
*
* Adresses the serial RAM chip and writes the given number of bytes
* from dataPtr_ui8 to the RAM, starting at address_ui16.
*
* @param address_ui16 RAM address to start writing to.
* @param dataPtr_pui8 Pointer to source.
* @param len_ui8 Number of bytes to be written
*/
void sram23k256_write(uint16_t address_ui16, uint8_t dataPtr_pui8[], uint8_t len_ui8)
{
  uint16_t ctr = 0;

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* send command & address */
  spi_send(SRAM_23K256_WRITE);
  spi_send(HI8(address_ui16));
  spi_send(LO8(address_ui16));

  /* Write data to chip */
  for (ctr = 0; ctr < len_ui8; ctr++)
  {
     spi_send(dataPtr_pui8[ctr]);
  }

  /* Release device */
  PIN_SET(SPI_CS_23K256);
}

/**
* @brief Initialization during boot-up
*
* Configures the serial RAM chip, clears the memory
* and performs a RAM test if configured.
* This method is called during boot up of ethersex.
*
* @param void
*/
int16_t sram23k256_init(void)
{
  uint16_t ctr = 0;
#ifdef SER_RAM_23K256_RAMTEST
  uint8_t data = 0;
  bool fail = false;
#endif

  PIN_SET(SRAM_23K256_HOLD);

  SERRAMDEBUG ("init\n");

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* send command & configuration byte (do not use HOLD, Sequential access) */
  spi_send(SRAM_23K256_WRSR);
  spi_send(SRAM_23K256_HOLD|SRAM_23K256_MODE_SEQ);

  /* Release device */
  PIN_SET(SPI_CS_23K256);

#ifdef SER_RAM_23K256_RAMTEST

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* Send command & start address (0) */
  spi_send(SRAM_23K256_WRITE);
  spi_send(0);
  spi_send(0);

  /* Write test data into RAM */
  for (ctr = 0; ctr < SRAM23K256_SIZE; ctr++)
  {
    spi_send(data);
    data++;
  }
  /* Release device */
  PIN_SET(SPI_CS_23K256);

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* Send command & start address (0) */
  spi_send(SRAM_23K256_READ);
  spi_send(0);
  spi_send(0);

  data = 0;
  /* Read & compare test data from RAM */
  for (ctr = 0; ctr < SRAM23K256_SIZE; ctr++)
  {
    if (data != spi_send(0))
    {
      /* Test data read did not match with what should be there */
      fail = true;
      break;
    }
    data++;
  }

  /* Release device */
  PIN_SET(SPI_CS_23K256);

  if (true == fail)
  {
    SERRAMDEBUG ("RAM test failed!\n");
  }
  else
  {
    SERRAMDEBUG ("RAM test OK!\n");
  }
#endif

  /* Acquire device */
  PIN_CLEAR(SPI_CS_23K256);

  /* Send command & start address (0) */
  spi_send(SRAM_23K256_WRITE);
  spi_send(0);
  spi_send(0);

  /* Write all zeros into RAM */
  for (ctr = 0; ctr < SRAM23K256_SIZE; ctr++)
  {
    spi_send(0);
  }

  /* Release device */
  PIN_SET(SPI_CS_23K256);

  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(hardware/serial_ram/23k256/sram_23k256.h)
  initearly(sram23k256_init)
*/
#endif /* SER_RAM_23K256_SUPPORT */
