/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _I2C_PC9531_H
#define _I2C_PC9531_H

#define I2C_SLA_PCA9531 0x60

uint8_t i2c_pca9531_set(uint8_t address, uint8_t prescaler0, uint8_t pwm0duty, 
                        uint8_t prescaler1, uint8_t pwm1duty, uint8_t led0to3, 
                        uint8_t led4to7);

#endif /* _I2C_PC9531_H */
