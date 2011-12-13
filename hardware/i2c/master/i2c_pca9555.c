/*
 *
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

#ifdef I2C_PCA9555_SUPPORT

#define LOW_BYTE(_v) (0xFF & _v)
#define HIGH_BYTE(_v) ((0xFF00 & _v) >> 8)

bool i2c_pca9555_read(uint8_t address, uint8_t reg, uint16_t* value)
{
  uint8_t result;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 read");
#endif

  result = i2c_master_select(address, TW_WRITE);

  if (1 == result)
  {
	  TWDR = reg;
	  if (TW_MT_DATA_ACK != i2c_master_transmit_with_ack())
	  {
		  result = 0;
	  }
  }

  if (1 == result)
  {
	  result = i2c_master_select(address, TW_READ);
  }

  if (1 == result)
  {
	  if (TW_MR_DATA_ACK != i2c_master_transmit_with_ack())
	  {
		  result = 0;
	  }

	  *value = TWDR;
  }

  if (1 == result)
  {
	  if (TW_MR_DATA_NACK != i2c_master_transmit())
	  {
		  result = 0;
	  }

	  *value |= TWDR<<8;
  }

  i2c_master_stop();

  return result;
}

bool i2c_pca9555_write(uint8_t address, uint8_t reg, uint16_t value)
{
	uint8_t result = 1;

#ifdef DEBUG_I2C
  debug_printf("I2C: pca9555 write");
#endif

  result = i2c_master_select(address, TW_WRITE);

  if (1 == result)
  {
	  TWDR = reg;
	  if (TW_MT_DATA_ACK != i2c_master_transmit_with_ack())
	  {
		  debug_printf(" failed @1\n");
		  result = 0;
	  }
  }

  if (1 == result)
  {
	  TWDR = LOW_BYTE(value);
	  if (TW_MT_DATA_ACK != i2c_master_transmit_with_ack())
	  {
		  debug_printf(" failed@2\n");
		  result = 0;
	  }
  }

  if (1 == result)
  {
	  TWDR = HIGH_BYTE(value);
	  if (TW_MT_DATA_ACK != i2c_master_transmit_with_ack())
	  {
		  debug_printf(" failed@3\n");
		  result = 0;
	  }
  }

#ifdef DEBUG_I2C
  if (1 == result)
  {
	  debug_printf("I2C: pca9555 write value: %X OK\n",value);
  }
  else
  {
	  debug_printf("I2C: pca9555 write failed\n",value);
  }
#endif

  i2c_master_stop();
  return result;
}

#endif /* I2C_PCA9555_SUPPORT */
