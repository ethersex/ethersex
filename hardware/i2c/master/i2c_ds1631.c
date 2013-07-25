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

#include <avr/io.h>
#include <util/twi.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"

#ifdef I2C_DS1631_SUPPORT

uint16_t i2c_ds1631_start_stop(const uint8_t chipaddress, const uint8_t startstop)
{
	uint16_t ret = 0xffff;

	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		goto end;
	}

	TWDR = (startstop == 0 ? 0x22 : 0x51); // start/stop convert T command

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		goto end;
	}

	ret = 0x0;

end:
	i2c_master_stop();
	return ret;
}

uint16_t i2c_ds1631_read_temperature(const uint8_t chipaddress, int16_t *temp, int16_t *stemp)
{
	int16_t data[2];
	uint16_t ret = 0xffff;

	if (!i2c_master_select(chipaddress, TW_WRITE))
	{
		goto end;
	}

	TWDR = 0xAA; // read last converted temperature

	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
	{
		goto end;
	}

	/* Do an repeated start condition */
	if (i2c_master_start() != TW_REP_START)
	{
		goto end;
	}

	TWDR = (chipaddress << 1) | TW_READ;

	if (i2c_master_transmit() != TW_MR_SLA_ACK)
	{
		goto end;
	}

	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
	{
		goto end;
	}

	data[1] = TWDR;

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_ds1631_read_temp: msb 0x%X\n",data[1]);
#endif

	if (i2c_master_transmit() != TW_MR_DATA_NACK)
	{
		goto end;
	}

	data[0] = TWDR;

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_ds1631_read_temp: lsb 0x%X\n",data[0]);
#endif

	if (data[1] & 0x80)
	{
		*temp = (128 - (data[1] & 0x7F));
		*temp *= -1;
	}
	else
	{
		*temp = data[1];
	}

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_ds1631_read_temp: temp 1: %d\n",*temp);
#endif

	*stemp = ((data[0] >> 4) * 625);

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_ds1631_read_temp: temp: %d.%d\n",*temp,*stemp);
#endif

	ret = 0x0;

end:
	i2c_master_stop();
	return ret;
}

#endif /* I2C_DS1631_SUPPORT */
