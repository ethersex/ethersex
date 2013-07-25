/*
 *
 * Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#ifdef I2C_PCA9685_SUPPORT
#include "config.h"
#define MODE1 0x00
#define MODE2 0x01
#define SUBADR1 0x02
#define SUBADR2 0x03
#define SUBADR  0x04
#define ALLCALLADR 0x05
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define ALL_LED_ON_L 0xFA
#define ALL_LED_ON_H 0xFB
#define ALL_LED_OFF_L 0xFC
#define ALL_LED_OFF_H 0xFD
#define PRE_SCALE 0xFE
uint8_t i2c_pca9685_reset(void);
uint8_t i2c_pca9685_set_mode(uint8_t address, uint8_t outdrv, uint8_t ivrt,
                             uint8_t prescaler);
uint8_t i2c_pca9685_set_led(uint8_t address, uint8_t led, uint16_t on,
                            uint16_t off);
uint8_t i2c_pca9685_set_leds(uint8_t address, uint8_t startled, uint8_t count,
                             uint16_t * values);
uint8_t i2c_pca9685_set_leds_fast(uint8_t address, uint8_t startled,
                                  uint8_t count, uint16_t * values);
#ifdef PCA9685_OUTPUT_ENABLE
enum i2c_pca9685_output_enable_state
{ ON, OFF, TOGGLE };
void i2c_pca9685_output_enable(enum i2c_pca9685_output_enable_state choice);
#endif
#endif
