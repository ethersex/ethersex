/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Klaus Gleißner  <mail@KlausGleissner.de>
 *
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
 }}} */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
        
#include "../config.h"
#include "../debug.h"
#include "i2c_master.h"

#ifdef I2C_LM75_SUPPORT

static uint8_t i2c_lm75_address;

void i2c_testaddress(uint8_t i2c_sla_address){
  uint8_t i;
  uint8_t i2c_address;

#ifdef DEBUG_I2C
  debug_printf("I2C: test for base address %X\n",i2c_sla_address);
#endif
  for (i = i2c_sla_address; i < i2c_sla_address + 8; i++) {
    if (i2c_master_select(i, TW_WRITE)) {
      i2c_address = i;
#ifdef DEBUG_I2C
      debug_printf("I2C: detected at: %X\n", i2c_address);
#endif
      i2c_master_stop();
      break;
    }
    i2c_master_stop();
  }
}

uint16_t
i2c_lm75_read_temp(uint8_t address){
  uint8_t temp[2];
  uint16_t ret;

#ifdef DEBUG_I2C
  debug_printf("I2C: lm75 read\n");
#endif
  i2c_master_select(address, TW_READ);

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) { ret = 0; goto end; }
  temp[0] = TWDR;
#ifdef DEBUG_I2C
  debug_printf("I2C: lm75 read value1: %d\n",temp1);
#endif

  if (i2c_master_transmit() != TW_MR_DATA_NACK) { ret = 0; goto end; }
  temp[1] = (TWDR & 0x80);
#ifdef DEBUG_I2C
  debug_printf("I2C: lm75 read value2: %d\n",temp2);
#endif
	
  ret = ( (temp[0] << 8) | temp[1] ) / 128*5;
end:
  i2c_master_stop();
  return ret;
}

#endif /* I2C_LM75_SUPPORT */
