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

#ifndef _I2C_PCA9555_H
#define _I2C_PCA9555_H

#define I2C_SLA_PCA9555 0x20

#define I2C_PCA9555_REG_IN 0
#define I2C_PCA9555_REG_OUT 2
#define I2C_PCA9555_REG_POL 4
#define I2C_PCA9555_REG_CONF 6

int8_t i2c_pca9555_read(uint8_t address, uint8_t reg, uint16_t* value);
int8_t i2c_pca9555_write(uint8_t address, uint8_t reg, uint16_t value);

#define i2c_pca9555_setDDR(address, value) i2c_pca9555_write(address, I2C_PCA9555_REG_CONF,  value)
#define i2c_pca9555_getDDR(address, valuePtr) i2c_pca9555_read(address, I2C_PCA9555_REG_CONF, valuePtr)

#define i2c_pca9555_setPolarity(address, value) i2c_pca9555_write(address, I2C_PCA9555_REG_POL,  value)
#define i2c_pca9555_getPolarity(address, valuePtr) i2c_pca9555_read(address, I2C_PCA9555_REG_POL, valuePtr)

#define i2c_pca9555_readPort(address, value) i2c_pca9555_read(address, I2C_PCA9555_REG_IN,  data) //set output word
#define i2c_pca9555_writePort(address, valuePtr) i2c_pca9555_write(address, I2C_PCA9555_REG_OUT,  valuePtr) //set output word

#endif /* _I2C_PCA9555_H */
