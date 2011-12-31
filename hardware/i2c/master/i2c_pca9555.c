/*
 * Copyright (c) 2011 by Daniel Walter <fordprfkt@googlemail.com>
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
#include <util/twi.h>
#include <stdbool.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_pca9555.h"

#ifdef I2C_PCA9555_SUPPORT

#define LOW_BYTE(_v) (0xFF & _v)
#define HIGH_BYTE(_v) ((0xFF00 & _v) >> 8)

uint8_t i2c_pca9555_readWord(uint8_t addrOffset, uint8_t reg, uint16_t* value)
{
  uint8_t result;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 read word ");
#endif

  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset, TW_WRITE);

  if (1 == result)
  {
    TWDR = reg;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

  if (1 == result)
  {
    result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset, TW_READ);
  }

  if (1 == result)
  {
    result = (TW_MR_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
    *value = TWDR;
  }

  if (1 == result)
  {
    result = (TW_MR_DATA_NACK == i2c_master_transmit()) ? 1 : 0;
    *value |= (uint16_t) TWDR << 8;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("%i OK.\n", *value);
  }
  else
  {
    debug_printf("failed.\n");
  }
#endif

  i2c_master_stop();

  return result;
}

uint8_t i2c_pca9555_readByte(uint8_t addrOffset, uint8_t reg, uint8_t* value)
{
  uint8_t result;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 read byte ");
#endif

  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset, TW_WRITE);

  if (1 == result)
  {
    TWDR = reg;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

  if (1 == result)
  {
    /* Do an repeated start condition */
    result = (i2c_master_start() == TW_REP_START) ? 1 : 0;

    if (1 == result)
    {
      /* Send the address again */
      TWDR = (I2C_SLA_PCA9555 << 1) | TW_READ;
      result = (i2c_master_transmit() == TW_MR_SLA_ACK) ? 1 : 0;
    }
  }

  if (1 == result)
  {
    result = (TW_MR_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
    *value = TWDR;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("%i OK.\n", *value);
  }
  else
  {
    debug_printf("failed.\n");
  }
#endif

  i2c_master_stop();

  return result;
}

uint8_t i2c_pca9555_writeWord(uint8_t addrOffset, uint8_t reg, uint16_t value)
{
  uint8_t result = 1;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 write word ");
#endif

  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset, TW_WRITE);

  if (1 == result)
  {
    TWDR = reg;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

  if (1 == result)
  {
    TWDR = LOW_BYTE(value);
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

  if (1 == result)
  {
    TWDR = HIGH_BYTE(value);
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("%i OK.\n", value);
  }
  else
  {
    debug_printf("failed\n");
  }
#endif

  i2c_master_stop();
  return result;
}

uint8_t i2c_pca9555_writeByte(uint8_t addrOffset, uint8_t reg, uint8_t value)
{
  uint8_t result = 1;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 write byte ");
#endif

  result = i2c_master_select(I2C_SLA_PCA9555 + addrOffset, TW_WRITE);

  if (1 == result)
  {
    TWDR = reg;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

  if (1 == result)
  {
    TWDR = value;
    result = (TW_MT_DATA_ACK == i2c_master_transmit_with_ack()) ? 1 : 0;
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
    debug_printf("%i OK.\n", value);
  }
  else
  {
    debug_printf("failed\n");
  }
#endif

  i2c_master_stop();
  return result;
}

#endif /* I2C_PCA9555_SUPPORT */
