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
#include <util/delay.h>

#include "config.h"
#include "autoconf.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_generic.h"

#ifdef I2C_GENERIC_SUPPORT

uint8_t i2c_read_byte(uint8_t chipaddress)
{
	uint8_t data[2];
	uint8_t ret;

#ifdef DEBUG_I2C
	debug_printf("I2C: read_byte: 0x%X (%d)\n",chipaddress,chipaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_READ))
	{
		ret = 0xff;
		goto end;
	}

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
	{
		ret = 0xff;
		goto end;
	}
	data[0] = TWDR;
#ifdef DEBUG_I2C
	debug_printf("I2C: read_byte value1: %X\n", data[0]);
#endif

	if (i2c_master_transmit() != TW_MR_DATA_NACK)
	{
		ret = 0xff;
		goto end;
	}
	data[1] = TWDR;
#ifdef DEBUG_I2C
	debug_printf("I2C: read_byte value2: %X\n", data[1]);
#endif

	ret = data[0];

	end: i2c_master_stop();
	return ret;
}

uint8_t i2c_read_byte_data(uint8_t chipaddress, uint8_t dataaddress)
{
	uint8_t ret = 0xff;

#ifdef DEBUG_I2C
	debug_printf("I2C: read_byte_data: 0x%X (%d) [0x%X (%d)]\n",
			chipaddress,chipaddress,dataaddress,dataaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		ret = 0xff;
		goto end;
	}

	TWDR = dataaddress;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xff;
		goto end;
	}

	/* Do an repeated start condition */
	if (i2c_master_start() != TW_REP_START)
	{
		ret = 0xff;
		goto end;
	}

	TWDR = (chipaddress << 1) | TW_READ;

	if (i2c_master_transmit() != TW_MR_SLA_ACK)
	{
		ret = 0xff;
		goto end;
	}

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
	{
		ret = 0xff;
		goto end;
	}

	ret = TWDR;

	end: i2c_master_stop();

#ifdef DEBUG_I2C
	debug_printf("I2C: read_byte_data value: 0x%X (%d)\n",ret,ret);
#endif

	return ret;
}

uint16_t i2c_read_word_data(uint8_t chipaddress, uint8_t dataaddress)
{
	uint8_t data[2];
	uint16_t ret = 0xffff;

#ifdef DEBUG_I2C
	debug_printf("I2C: read_word_data: 0x%X [0x%X]\n",chipaddress,dataaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		ret = 0xffff;
		goto end;
	}

	TWDR = dataaddress;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

#ifdef I2C_GENERIC_DELAYS
	_delay_ms(10); // for slow devices
#endif

#ifdef DEBUG_I2C
	debug_printf("I2C: repeated start\n");
#endif

	/* Do an repeated start condition */
	if (i2c_master_start() != TW_REP_START)
	{
		ret = 0xffff;
		goto end;
	}

	TWDR = (chipaddress << 1) | TW_READ;

	if (i2c_master_transmit() != TW_MR_SLA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

#ifdef I2C_GENERIC_DELAYS
	_delay_ms(10); // for slow devices
#endif

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}
	data[0] = TWDR;

#ifdef DEBUG_I2C
	debug_printf("I2C: data0: %d 0x%x\n",data[0],data[0]);
#endif

#ifdef I2C_GENERIC_DELAYS
	_delay_ms(10); // for slow devices
#endif

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

	data[1] = TWDR;

#ifdef DEBUG_I2C
	debug_printf("I2C: data1: %d 0x%x\n",data[1],data[1]);
#endif

	ret = data[0] << 8 | data[1];

	end: i2c_master_stop();
	return ret;
}

uint16_t i2c_write_byte(uint8_t chipaddress, uint8_t data)
{
	uint16_t ret = 0xffff;

#ifdef DEBUG_I2C
	debug_printf("I2C: write_byte: 0x%X (%d)\n",chipaddress,chipaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		ret = 0xffff;
		goto end;
	}

#ifdef DEBUG_I2C
	debug_printf("I2C: data: 0x%X\n",data);
#endif

	TWDR = data;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

	ret = data;

	end: i2c_master_stop();
	return ret;
}

uint16_t i2c_write_byte_data(uint8_t chipaddress, uint8_t dataaddress, uint8_t data)
{
	uint16_t ret = 0xffff;

#ifdef DEBUG_I2C
	debug_printf("I2C: write_byte_data: 0x%X [0x%X]\n",chipaddress,dataaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		ret = 0xffff;
		goto end;
	}

	TWDR = dataaddress;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

#ifdef DEBUG_I2C
	debug_printf("I2C: data: 0x%X\n",data);
#endif

	TWDR = data;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

	ret = data;

	end: i2c_master_stop();
	return ret;
}

uint16_t i2c_write_word_data(uint8_t chipaddress, uint8_t dataaddress, uint16_t data)
{
	uint16_t ret = 0xffff;

#ifdef DEBUG_I2C
	debug_printf("I2C: write_word_data: 0x%X [0x%X]\n",chipaddress,dataaddress);
#endif
	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		ret = 0xffff;
		goto end;
	}

	TWDR = dataaddress;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

#ifdef DEBUG_I2C
	debug_printf("I2C: data: 0x%X\n",data);
#endif

	TWDR = (data >> 8) & 0xff;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

	TWDR = data & 0xff;

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		ret = 0xffff;
		goto end;
	}

	ret = data;

	end: i2c_master_stop();
	return ret;
}

#endif /* I2C_GENERIC_SUPPORT */
