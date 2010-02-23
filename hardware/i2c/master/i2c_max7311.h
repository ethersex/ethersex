/*
*
* Copyright (c) 2010 by Jens Prager <jprager@gmx.de>
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

#ifndef _I2C_MAX7311_H
#define _I2C_MAX7311_H

#define I2C_SLA_MAX7311 0x00

//-- Register --
#define MAX7311_IN_L     0x0
#define MAX7311_IN_H     0x1
#define MAX7311_OUT_L    0x2
#define MAX7311_OUT_H    0x3
#define MAX7311_POLINV_L 0x4
#define MAX7311_POLINV_H 0x5
#define MAX7311_DDR_L    0x6
#define MAX7311_DDR_H    0x7
#define MAX7311_TO_REG   0x8

uint8_t i2c_max7311_writeReg(uint8_t address, uint8_t reg,  uint16_t data);
uint8_t i2c_max7311_readReg(uint8_t address, uint8_t reg, uint16_t *data);

#define i2c_max7311_setDDRw(address, data) i2c_max7311_writeReg(address, MAX7311_DDR_L,  data) //write data direction register
#define i2c_max7311_setOUTw(address, data) i2c_max7311_writeReg(address, MAX7311_OUT_L,  data) //set output word
#define i2c_max7311_getDDRw(address, pnt_data) i2c_max7311_readReg(address, MAX7311_DDR_L, pnt_data) //read data direction register
#define i2c_max7311_getOUTw(address, pnt_data) i2c_max7311_readReg(address, MAX7311_OUT_L, pnt_data) //read output word
#define i2c_max7311_getINw(address, pnt_data) i2c_max7311_readReg(address, MAX7311_IN_L, pnt_data) //read input word

uint8_t i2c_max7311_set(uint8_t address, uint8_t port, uint8_t state); //set output bit port
uint8_t i2c_max7311_pulse(uint8_t address, uint8_t port, uint16_t time); //pulse output bit by inverting for time in ms 

#endif /* _I2C_MAX7311_H */
