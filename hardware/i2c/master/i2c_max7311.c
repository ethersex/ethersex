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

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_max7311.h"

#ifdef I2C_MAX7311_SUPPORT

uint8_t i2c_max7311_writeReg(uint8_t address, uint8_t reg,  uint16_t data)
{
	uint8_t ret = 0xFF;
	if (!i2c_master_select(address, TW_WRITE)) goto end;
	TWDR = reg;
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;
	TWDR = (unsigned char)(data & 0xFF);
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;
	TWDR = (unsigned char)((data >> 8) & 0xFF);
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;
#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_max7311_writeReg: Reg 0x%X: 0x%X, Reg 0x%X: 0x%X\n", reg, (data & 0xFF), reg+1, (unsigned char)((data >> 8) & 0xFF));
#endif
	ret = 0;
	end: i2c_master_stop();
	return ret;
}

uint8_t i2c_max7311_readReg(uint8_t address, uint8_t reg, uint16_t *data)
{
	uint8_t ret = 0xFF;
	uint8_t tmp[2];
	if (!i2c_master_select(address, TW_WRITE)) goto end;
	TWDR = reg;
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;
	/* Do an repeated start condition */
	if (i2c_master_start() != TW_REP_START) goto end;
	TWDR = (address << 1) | TW_READ;
	if (i2c_master_transmit() != TW_MR_SLA_ACK) goto end;

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) goto end;
	tmp[0] = TWDR;
	if (i2c_master_transmit() != TW_MR_DATA_NACK) goto end;
	tmp[1] = TWDR;
#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_max7311_readReg: Reg 0x%X: 0x%X, Reg 0x%X: 0x%X\n", reg, tmp[0], reg+1, tmp[1]);
#endif
	ret = 0;
	*data = ((tmp[1] << 8) | tmp[0]);
	end: i2c_master_stop();
	return ret;
}

uint8_t i2c_max7311_set(uint8_t address, uint8_t port, uint8_t state)
{
	uint8_t ret = 0xFF;
	uint16_t tmp;
	if (i2c_max7311_getOUTw(address, &tmp) != 0) goto end;
	tmp = (state ? tmp | (1 << port) : tmp & ~(1 << port));
	if (i2c_max7311_setOUTw(address, tmp) != 0) goto end;
	ret = 0;
	end: return ret;
}

uint8_t i2c_max7311_pulse(uint8_t address, uint8_t port, uint16_t time) 
{
	uint16_t tmp;
	uint8_t ret = 0xFF;
	if (i2c_max7311_getOUTw(address, &tmp) != 0) goto end;
	if (i2c_max7311_setOUTw(address, tmp ^ (1 << port)) != 0) goto end;
	while(time--)
		_delay_ms(1);
	if (i2c_max7311_setOUTw(address, tmp) != 0) goto end;
	ret = 0;
	end: return ret;
}

#endif /* I2C_MAX7311_SUPPORT */
