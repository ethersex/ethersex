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
#include <util/delay.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_tsl2550.h"

#ifdef I2C_TSL2550_SUPPORT

/*
 * Defines
 */

#define TSL2550_POWER_DOWN              0x00
#define TSL2550_POWER_UP                0x03
#define TSL2550_STANDARD_RANGE          0x18
#define TSL2550_EXTENDED_RANGE          0x1d
#define TSL2550_READ_ADC0               0x43
#define TSL2550_READ_ADC1               0x83

/*
 * Structs
 */

struct tsl2550_data
{
	unsigned int power_state :1;
	unsigned int operating_mode :1;
};

/*
 * Global data
 */

static const uint8_t TSL2550_MODE_RANGE[2] PROGMEM =
{ TSL2550_STANDARD_RANGE, TSL2550_EXTENDED_RANGE };

static struct tsl2550_data data;

/*
 * LUX calculation
 */

#define TSL2550_MAX_LUX 1846

static const uint8_t ratio_lut[129] PROGMEM =
{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
		99, 98, 98, 98, 98, 98, 98, 98, 97, 97, 97, 97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 93, 93, 93, 92, 92, 91,
		91, 90, 89, 89, 88, 87, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 75, 74, 73, 71, 69, 68, 66, 64, 62, 60, 58,
		56, 54, 52, 49, 47, 44, 42, 41, 40, 40, 39, 39, 38, 38, 37, 37, 37, 36, 36, 36, 35, 35, 35, 35, 34, 34, 34, 34,
		33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, };

static const uint16_t count_lut[128] PROGMEM =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46,
		49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 115, 123, 131, 139, 147, 155, 163, 171, 179,
		187, 195, 203, 211, 219, 227, 235, 247, 263, 279, 295, 311, 327, 343, 359, 375, 391, 407, 423, 439, 455, 471,
		487, 511, 543, 575, 607, 639, 671, 703, 735, 767, 799, 831, 863, 895, 927, 959, 991, 1039, 1103, 1167, 1231,
		1295, 1359, 1423, 1487, 1551, 1615, 1679, 1743, 1807, 1871, 1935, 1999, 2095, 2223, 2351, 2479, 2607, 2735,
		2863, 2991, 3119, 3247, 3375, 3503, 3631, 3759, 3887, 4015, };

uint16_t tsl2550_compute_lux(const uint8_t adc0, const uint8_t adc1)
{
	/* Look up count from channel values */
	uint32_t c0 = 0;
	uint32_t c1 = 0;
	uint8_t ratio = 128; // Note: the "128" is a scaling factor
	uint16_t lux = 0;

	/*
	 * Calculate ratio.
	 *
	 */

#ifdef DEBUG_I2C
	debug_printf("I2C: tsl2550: adc0=%d, adc1=%d\n", adc0, adc1);
#endif

	c0 = pgm_read_word(&count_lut[adc0]);
	c1 = pgm_read_word(&count_lut[adc1]);

#ifdef DEBUG_I2C
	debug_printf("I2C: tsl2550: c0=%ld, c1=%ld\n",c0,c1);
#endif

	/* Avoid division by 0 and count 1 cannot be greater than count 0 */
	if (c1 <= c0)
	{
		if (c0)
		{
			/* Calculate LUX */
			ratio = (c1 * 128ul) / c0;
			/* the "256" is a scaling factor */
			lux = ((c0 - c1) * pgm_read_byte(&ratio_lut[ratio])) / 256;

#ifdef DEBUG_I2C
			debug_printf("I2C: tsl2550: ratio=%d, ratio_lut[ratio]=%d\n",
					ratio,pgm_read_byte(&ratio_lut[ratio]));
			debug_printf("I2C: tsl2550: c0-c1=%d\n",(c0-c1));
#endif
		}
	}

	return lux > TSL2550_MAX_LUX ? TSL2550_MAX_LUX : lux;
}

uint16_t i2c_tsl2550_set_operating_mode(const uint8_t mode)
{
	uint16_t ret = 0xffff;


	if (!i2c_master_select(I2C_SLA_TSL2550, TW_WRITE))
		goto end;

	TWDR = pgm_read_byte(&TSL2550_MODE_RANGE[mode]);
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
		goto end;

	data.operating_mode = mode;
	ret = mode;

#ifdef DEBUG_I2C
	debug_printf("I2C: tsl2550: op mode: %d\n",mode);
#endif

	end: i2c_master_stop();
	return ret;
}

uint16_t i2c_tsl2550_set_power_state(const uint8_t state)
{
	uint16_t ret = 0xffff;

	if (!i2c_master_select(I2C_SLA_TSL2550, TW_WRITE))
		goto end;

	if (state == 0)
	{
		TWDR = TSL2550_POWER_DOWN;
		if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
			goto end;
	}
	else
	{
		TWDR = TSL2550_POWER_UP;
		if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
			goto end;
		/* On power up we should reset operating mode also... */
		i2c_tsl2550_set_operating_mode(data.operating_mode);
	}

	data.power_state = state;
	ret = state;

#ifdef DEBUG_I2C
	debug_printf("I2C: tsl2550: pwr state: %d\n",state);
#endif

	end: i2c_master_stop();
	return ret;
}

uint16_t __tsl2550_read_adc(const uint8_t adc)
{
	uint16_t val = 0xffff;
	uint8_t data[2];

	if (!i2c_master_select(I2C_SLA_TSL2550, TW_WRITE)) goto end;

	TWDR = (adc == 0 ? TSL2550_READ_ADC0 : TSL2550_READ_ADC1);
	if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;

	i2c_master_stop();

	if (!i2c_master_select(I2C_SLA_TSL2550, TW_READ)) goto end;
	if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) goto end;

	data[0] = TWDR;
#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_tsl2550_read_lux: data0: 0x%X\n",data[0]);
#endif

	if (i2c_master_transmit() != TW_MR_DATA_NACK) goto end;

	data[1] = TWDR;
#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_tsl2550_read_lux: data1: 0x%X\n",data[1]);
#endif

	val = data[0];

	end: i2c_master_stop();

	return val;
}

uint8_t tsl2550_read_adc(const uint8_t adc)
{
	uint16_t ret = 0xff;
	uint8_t loop = 0;

	while ( loop < 5 )
	{
		loop++;

		ret = __tsl2550_read_adc(adc);

		if ( (ret != 0xffff) && ((ret & 0x80) == 0x80) )
			break;

#ifdef DEBUG_I2C
		debug_printf("I2C: loop\n");
#endif

		_delay_ms(5);
	}

	return ret & 0x7f;
}

uint16_t i2c_tsl2550_show_lux_level(void)
{
	uint8_t adc[2];
	uint16_t ret = 0xffff;

	/* read adc channel 0 */
	adc[0] = tsl2550_read_adc(0);

	_delay_ms(2);

	/* read adc channel 1 */
	adc[1] = tsl2550_read_adc(1);

	/* compute lux */
	ret = tsl2550_compute_lux(adc[0], adc[1]);

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_tsl2550_read_lux: mode: %d, lux: %d\n",data.operating_mode,ret);
#endif

	if (data.operating_mode == 1)
	{
		ret *= 5;
#ifdef DEBUG_I2C
		debug_printf("I2C: i2c_tsl2550_read_lux: %d\n",ret);
#endif
	}

	return ret;
}

void i2c_tsl2550_init(void)
{
	data.power_state = 0;
	data.operating_mode = 0;

	//TODO: detect TSL2550

	//i2c_tsl2550_set_power_state(1);
	//i2c_tsl2550_set_operating_mode(1);
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_tsl2550.h)
 init(i2c_tsl2550_init)
 */

#endif /* I2C_TSL2550_SUPPORT */
