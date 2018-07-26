/*
 * Copyright (c) 2018 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __I2C_BMP280_H
#define __I2C_BMP280_H

#include <stdint.h>

#include "config.h"

/* ODR options */
#define BMP280_ODR_0_5_MS      0x00
#define BMP280_ODR_62_5_MS     0x01
#define BMP280_ODR_125_MS      0x02
#define BMP280_ODR_250_MS      0x03
#define BMP280_ODR_500_MS      0x04
#define BMP280_ODR_1000_MS     0x05
#define BMP280_ODR_2000_MS     0x06
#define BMP280_ODR_4000_MS     0x07

/* Over-sampling */
#define BMP280_OS_NONE         0x00
#define BMP280_OS_1X           0x01
#define BMP280_OS_2X           0x02
#define BMP280_OS_4X           0x03
#define BMP280_OS_8X           0x04
#define BMP280_OS_16X          0x05

/* Filter coefficient */
#define BMP280_FILTER_OFF      0x00
#define BMP280_FILTER_COEFF_2  0x01
#define BMP280_FILTER_COEFF_4  0x02
#define BMP280_FILTER_COEFF_8  0x03
#define BMP280_FILTER_COEFF_16 0x04

/* Power modes */
#define BMP280_SLEEP_MODE      0x00
#define BMP280_FORCED_MODE     0x01
#define BMP280_NORMAL_MODE     0x03

#define BMP280_RESULT_OK        0
#define BMP280_RESULT_ERROR    -1
#define BMP280_RESULT_NODEV    -2
#define BMP280_RESULT_INVAL    -3


typedef struct
{
  uint8_t os_temp;
  uint8_t os_pres;
  uint8_t odr;
  uint8_t filter;
} i2c_bmp280_conf;


void i2c_bmp280_init(void);
int8_t i2c_bmp280_read_conf(i2c_bmp280_conf * conf);
int8_t i2c_bmp280_write_conf(uint8_t mode, const i2c_bmp280_conf * conf);
int8_t i2c_bmp280_set_power_mode(uint8_t mode);
int8_t i2c_bmp280_get_temp(int16_t * temp);
int8_t i2c_bmp280_get_press(uint16_t * press);

#endif /* __I2C_BMP280_H */
