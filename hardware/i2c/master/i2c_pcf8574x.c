/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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
        
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"

#ifdef I2C_PCF8574X_SUPPORT

int8_t
i2c_pcf8574x_read(uint8_t address){
  uint8_t data[2];
  uint8_t ret;

#ifdef DEBUG_I2C
  debug_printf("I2C: pcf8574X read\n");
#endif
  if (!i2c_master_select(address, TW_READ)) { ret = 0xff; goto end; }

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) { ret = 0xff; goto end; }
  data[0] = TWDR;
#ifdef DEBUG_I2C
  debug_printf("I2C: pcf8574X read value1: %X\n", data[0]);
#endif

  if (i2c_master_transmit() != TW_MR_DATA_NACK) { ret = 0xff; goto end; }
  data[1] = TWDR;
#ifdef DEBUG_I2C
  debug_printf("I2C: pcf8574X read value2: %X\n", data[1]);
#endif
	
  ret = data[0] ;
end:
  i2c_master_stop();
  return ret;
}

int16_t
i2c_pcf8574x_set(uint8_t address, uint8_t value){
  uint16_t ret=0xffff;

#ifdef DEBUG_I2C
  debug_printf("I2C: pcf8574X set\n");
#endif
  i2c_master_select(address, TW_WRITE);

#ifdef DEBUG_I2C
  debug_printf("I2C: pcf8574X: 0x%X", i2c_master_transmit_with_ack());
  debug_printf("I2C: pcf8574X read value: %X\n",value);
#endif
  TWDR=value;

  i2c_master_transmit();

  i2c_master_stop();
  return ret;
}

#endif /* I2C_PCF8574X_SUPPORT */
