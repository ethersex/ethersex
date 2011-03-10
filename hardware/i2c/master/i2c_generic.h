/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _I2C_GENERIC_H
#define _I2C_GENERIC_H

uint8_t i2c_read_byte(const uint8_t chipaddress);
uint8_t i2c_read_byte_data(const uint8_t chipaddress, const uint8_t dataaddress);
uint16_t i2c_read_word_data(const uint8_t chipaddress, const uint8_t dataaddress);

uint16_t i2c_write_byte(const uint8_t chipaddress, const uint8_t data);
uint16_t i2c_write_byte_data(const uint8_t chipaddress, const uint8_t dataaddress, const uint8_t data);
uint16_t i2c_write_word_data(const uint8_t chipaddress, const uint8_t dataaddress, const uint16_t data);

#endif /* _I2C_GENERIC_H */
