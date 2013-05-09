/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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
 */

#ifndef _I2C_EEPROM_I2C_MASTER_H
#define _I2C_EEPROM_I2C_MASTER_H

#define i2c_master_disable() TWCR = 0
#define i2c_master_enable() TWCR=(1<<TWEN)|(1<<TWINT)

void i2c_master_init(void);
uint8_t i2c_master_detect(uint8_t range_start, uint8_t range_end);

uint8_t i2c_master_do(uint8_t mode);
void i2c_master_stop(void);
#define i2c_master_start() i2c_master_do(_BV(TWINT) | _BV(TWEN) | _BV(TWSTA))

uint8_t  i2c_master_select(uint8_t address, uint8_t mode);
#define i2c_master_transmit() i2c_master_do(_BV(TWEN) | _BV(TWINT)) 
#define i2c_master_transmit_with_ack() i2c_master_do(_BV(TWEN) | _BV(TWINT) | _BV(TWEA) ) 

#include "config.h"
#ifdef DEBUG_I2C
# include "core/debug.h"
# define I2CDEBUG(a...)  debug_printf("i2c: " a)
#else
# define I2CDEBUG(a...)
#endif

#endif /* _I2C_EEPROM_I2C_MASTER_H */
