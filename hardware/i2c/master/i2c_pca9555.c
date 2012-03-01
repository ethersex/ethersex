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

#include <avr/io.h>
#include <core/bit-macros.h>
#include <util/twi.h>
#include <stdbool.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_pca9555.h"

#ifdef I2C_PCA9555_SUPPORT

/**
* @brief Read one word (16 bit) from the chip.
*
* Addresses the chip and reads both bytes from the given register into value_pui16.
* Note: The low byte of the desired register must be given in reg_ui8 or the bytes in
* the result will be reversed.
*
* @param[in] addrOffset_ui8 Offset to the slave address (0x20) of the chip (0 if not used).
* @param[in] reg_ui8 Register to be read
* @param[out] value_pui16 Pointer to data storage
*
* @result 0 = an error occurred, 1 = operation succeeded
*/
uint8_t
i2c_pca9555_readWord(uint8_t addrOffset_ui8, uint8_t reg_ui8,
                     uint16_t * value_pui16)
{
  uint8_t result;

  /* Address the chip for writing */
  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset_ui8, TW_WRITE);

  /* Send the desired register */
  if (1 == result)
  {
    TWDR = reg_ui8;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

  if (1 == result)
  {
    /* Make a repeated start */
    result = (i2c_master_start() == TW_REP_START) ? 1 : 0;

    if (1 == result)
    {
      /* Address the chip for reading */
      TWDR = (I2C_SLA_PCA9555 << 1) | TW_READ;
      result = (i2c_master_transmit() == TW_MR_SLA_ACK) ? 1 : 0;
    }
  }

  /* Read low byte */
  if (1 == result)
  {
    result = (TW_MR_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
    *value_pui16 = TWDR;
  }

  /* Read high byte */
  if (1 == result)
  {
    /* Do not acknowledge last byte */
    result = (TW_MR_DATA_NACK == i2c_master_transmit())? 1 : 0;
    *value_pui16 |= (uint16_t) TWDR << 8;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("I2C: pca9555 read word @%i OK. (%i)\n", reg_ui8,
                 *value_pui16);
  }
  else
  {
    debug_printf("I2C: pca9555 read word @%i failed.\n", reg_ui8);
  }
#endif

  i2c_master_stop();

  return result;
}

/**
* @brief Read one byte from the chip.
*
* Addresses the chip and reads the given register into value_pui8.
*
* @param[in] addrOffset_ui8 Offset to the slave address (0x20) of the chip (0 if not used).
* @param[in] reg_ui8 Register to be read
* @param[out] value_pui8 Pointer to data storage
*
* @result 0 = an error occurred, 1 = operation succeeded
*/
uint8_t
i2c_pca9555_readByte(uint8_t addrOffset_ui8, uint8_t reg_ui8,
                     uint8_t * value_pui8)
{
  uint8_t result;

  /* Address the chip for writing */
  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset_ui8, TW_WRITE);

  /* Send the desired register */
  if (1 == result)
  {
    TWDR = reg_ui8;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

  if (1 == result)
  {
    /* Make a repeated start */
    result = (i2c_master_start() == TW_REP_START) ? 1 : 0;

    if (1 == result)
    {
      /* Address the chip for reading */
      TWDR = (I2C_SLA_PCA9555 << 1) | TW_READ;
      result = (i2c_master_transmit() == TW_MR_SLA_ACK) ? 1 : 0;
    }
  }

  /* Read the register */
  if (1 == result)
  {
    result = (TW_MR_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
    *value_pui8 = TWDR;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("I2C: pca9555 read byte @%i OK. (%i)\n", reg_ui8,
                 *value_pui8);
  }
  else
  {
    debug_printf("I2C: pca9555 read byte @%i failed.\n", reg_ui8);
  }
#endif

  i2c_master_stop();

  return result;
}

/**
* @brief Send one word (16bit) to the chip.
*
* Addresses the chip and writes value_ui16 into the low
* byte and the high byte of the given register.
*
* @param[in] addrOffset_ui8 Offset to the slave address (0x20) of the chip (0 if not used).
* @param[in] reg_ui8 Register to be written.
* @param[in] value_ui16 Data to be written.
*
* @result 0 = an error occurred, 1 = operation succeeded
*/
uint8_t
i2c_pca9555_writeWord(uint8_t addrOffset_ui8, uint8_t reg_ui8,
                      uint16_t value_ui16)
{
  uint8_t result = 1;

  /* Address the chip for writing */
  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset_ui8, TW_WRITE);

  /* Send the desired register */
  if (1 == result)
  {
    TWDR = reg_ui8;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

  /* Send the low byte */
  if (1 == result)
  {
    TWDR = LO8(value_ui16);
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

  /* Send the high byte */
  if (1 == result)
  {
    TWDR = HI8(value_ui16);
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("I2C: pca9555 write word %i @%i OK.\n", value_ui16, reg_ui8);
  }
  else
  {
    debug_printf("I2C: pca9555 write word %i @%i failed\n", reg_ui8);
  }
#endif

  i2c_master_stop();
  return result;
}

/**
* @brief Send one byte to the chip.
*
* Addresses the chip and writes value_ui8 into the given register.
*
* @param[in] addrOffset_ui8 Offset to the slave address (0x20) of the chip (0 if not used).
* @param[in] reg_ui8 Register to be written.
* @param[in] value_ui8 Data to be written.
*
* @result 0 = an error occurred, 1 = operation succeeded
*/
uint8_t
i2c_pca9555_writeByte(uint8_t addrOffset_ui8, uint8_t reg_ui8,
                      uint8_t value_ui8)
{
  uint8_t result = 1;

  /* Address the chip for writing */
  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset_ui8, TW_WRITE);

  /* Send the desired register */
  if (1 == result)
  {
    TWDR = reg_ui8;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

  /* Send the value */
  if (1 == result)
  {
    TWDR = value_ui8;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack())? 1 : 0;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("I2C: pca9555 write byte %i @%i OK.\n", value_ui8, reg_ui8);
  }
  else
  {
    debug_printf("I2C: pca9555 write byte %i @%i failed\n", reg_ui8);
  }
#endif

  i2c_master_stop();
  return result;
}

#endif /* I2C_PCA9555_SUPPORT */
