/*
 *
 * Copyright (c) 2010 by
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

#ifndef _I2C_TSL2550_H
#define _I2C_TSL2550_H

#define I2C_SLA_TSL2550 0x39 // there can only be one!

uint16_t i2c_tsl2550_set_power_state(const uint8_t state); // 0: down, 1: up
uint16_t i2c_tsl2550_set_operating_mode(const uint8_t mode); // 0: standard, 1: extended range
uint16_t i2c_tsl2550_show_lux_level(void);
void i2c_tsl2550_init(void);

#endif /* _I2C_TSL2550_H */
