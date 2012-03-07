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

#ifndef _I2C_PCA9555_H
#define _I2C_PCA9555_H

#define I2C_SLA_PCA9555 0x20    /* Slave address of the PCA9555 */

#define I2C_PCA9555_REG_IN 0    /* Input register, low byte */
#define I2C_PCA9555_REG_OUT 2   /* Output register, low byte */
#define I2C_PCA9555_REG_POL 4   /* Polarity register, low byte */
#define I2C_PCA9555_REG_CONF 6  /* Configuration register, low byte */

uint8_t i2c_pca9555_readWord(uint8_t addrOffset, uint8_t reg,
                             uint16_t * value);
uint8_t i2c_pca9555_writeWord(uint8_t addrOffset, uint8_t reg,
                              uint16_t value);

uint8_t i2c_pca9555_readByte(uint8_t addrOffset, uint8_t reg,
                             uint8_t * value);
uint8_t i2c_pca9555_writeByte(uint8_t addrOffset, uint8_t reg, uint8_t value);

/* Set and get the data direction register for all pins */
#define i2c_pca9555_setDDR(addrOffset, value) i2c_pca9555_writeWord(addrOffset, I2C_PCA9555_REG_CONF,  value)
#define i2c_pca9555_getDDR(addrOffset, valuePtr) i2c_pca9555_readWord(addrOffset, I2C_PCA9555_REG_CONF, valuePtr)

/* Set and get the data direction register for the lower 8 pins */
#define i2c_pca9555_setDDRL(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_CONF,  value)
#define i2c_pca9555_getDDRL(addrOffset, valuePtr) i2c_pca9555_reaByte(addrOffset, I2C_PCA9555_REG_CONF, valuePtr)

/* Set and get the data direction register for the higher 8 pins */
#define i2c_pca9555_setDDRH(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_CONF+1,  value)
#define i2c_pca9555_getDDRH(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_CONF+1, valuePtr)

/* Set and get the polarity register for all pins */
#define i2c_pca9555_setPolarity(addrOffset, value) i2c_pca9555_writeWord(addrOffset, I2C_PCA9555_REG_POL,  value)
#define i2c_pca9555_getPolarity(addrOffset, valuePtr) i2c_pca9555_readWord(addrOffset, I2C_PCA9555_REG_POL, valuePtr)

/* Set and get the polarity register for the lower 8 pins */
#define i2c_pca9555_setPolarityL(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_POL,  value)
#define i2c_pca9555_getPolarityL(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_POL, valuePtr)

/* Set and get the polarity register for the higher 8 pins */
#define i2c_pca9555_setPolarityH(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_POL+1,  value)
#define i2c_pca9555_getPolarityH(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_POL+1, valuePtr)

/* Read the input register for all pins */
#define i2c_pca9555_readInPort(addrOffset, valuePtr) i2c_pca9555_readWord(addrOffset, I2C_PCA9555_REG_IN,  valuePtr)

/* Read and write the output register for all pins */
#define i2c_pca9555_readOutPort(addrOffset, valuePtr) i2c_pca9555_readWord(addrOffset, I2C_PCA9555_REG_OUT,  valuePtr)
#define i2c_pca9555_writeOutPort(addrOffset, value) i2c_pca9555_writeWord(addrOffset, I2C_PCA9555_REG_OUT,  value)

/* Read the input register for the lower 8 pins */
#define i2c_pca9555_readInPortL(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_IN,  valuePtr)

/* Read the input register for the higher 8 pins */
#define i2c_pca9555_readInPortH(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_IN+1,  valuePtr)

/* Read the output register for the lower 8 pins */
#define i2c_pca9555_readOutPortL(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_OUT,  valuePtr)

/* Read the output register for the higher 8 pins */
#define i2c_pca9555_readOutPortH(addrOffset, valuePtr) i2c_pca9555_readByte(addrOffset, I2C_PCA9555_REG_OUT+1,  valuePtr)

/* Write the output register for the lower 8 pins */
#define i2c_pca9555_writeOutPortL(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_OUT,  value)

/* Write the output register for the higher 8 pins */
#define i2c_pca9555_writeOutPortH(addrOffset, value) i2c_pca9555_writeByte(addrOffset, I2C_PCA9555_REG_OUT+1,  value)

#endif /* _I2C_PCA9555_H */
