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

#ifndef __I2C_BH1750_H
#define __I2C_BH1750_H

#include <stdint.h>

#include "config.h"

#define BH1750_RESULT_OK        0
#define BH1750_RESULT_ERROR    -1
#define BH1750_RESULT_NODEV    -2
#define BH1750_RESULT_INVAL    -3

#define RESOLUTION_LOW         1U
#define RESOLUTION_NORMAL      2U
#define RESOLUTION_HIGH        3U
#define RESOLUTION_AUTO_HIGH  99U

typedef uint8_t i2c_bh1750_resolution;

int8_t i2c_bh1750_set_operating_mode(const i2c_bh1750_resolution resolution,
                                     const uint8_t auto_power_down);
int32_t i2c_bh1750_get_lux(void);

#endif /* __I2C_BH1750_H */
