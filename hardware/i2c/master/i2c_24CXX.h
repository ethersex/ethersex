/*
 *
 * Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _I2C_EEPROM_I2C_24CXX_H
#define _I2C_EEPROM_I2C_24CXX_H

#define I2C_SLA_24CXX 80

void i2c_24CXX_init(void);
uint8_t i2c_24CXX_set_addr(uint16_t addr);

uint8_t i2c_24CXX_write_byte(uint16_t addr, uint8_t data);
uint8_t i2c_24CXX_write_block(uint16_t addr, uint8_t *ptr, uint8_t len);

#define i2c_24CXX_read_byte(addr, data) i2c_24CXX_read_block(addr, data, 1)
uint8_t i2c_24CXX_read_block(uint16_t addr, uint8_t *ptr, uint8_t len);

uint8_t i2c_24CXX_compare_block(uint16_t addr, uint8_t *ptr, uint8_t len);

#endif /* _I2C_EEPROM_I2C_24CXX_H */
