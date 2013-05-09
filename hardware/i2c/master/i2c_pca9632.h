/*
*
* Copyright (c) 2013 by Michael Schopferer <michael@schopferer.name>
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

#ifndef _I2C_PC9632_H
#define _I2C_PC9632_H

uint8_t i2c_pca9632_reset(void);
uint8_t i2c_pca9632_init(uint8_t address, uint8_t mode1, uint8_t mode2, 
                        uint8_t led_out_state);
uint8_t i2c_pca9632_set_blink(uint8_t address, uint8_t grppwm,
                             uint8_t grpfreq);
uint8_t i2c_pca9632_set_led(uint8_t address, uint8_t led_x, uint8_t pwm_x);
uint8_t i2c_pca9632_read_led(uint8_t address, uint8_t led_x);

#endif /* _I2C_PC9632_H */
