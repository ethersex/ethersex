/* 
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include "config.h"
#include "i2c_master.h"
#include "i2c_pca9685.h"
#ifdef I2C_PCA9685_SUPPORT

/*The PCA9685 features two operation modes: totem-pole and open-drain. Please refer to the datasheet which one suits your
  setup. (Page 28, Table 11)
  | ---------------------------------------------------------------- |
  | option  | v | impact                                             |
  | ---------------------------------------------------------------- |
  | outdrv  | 0 | open-drain                                         |  					
  | outdrv  | 1 | totem-pole                                         |
  | ivrt    | 0 | normal pwm output   (LED ON means PIN is high)     |
  | ivrt    | 1 | inverted pwm output (LED ON means PIN is low)      |
  | ---------------------------------------------------------------- |
 */
#define LOW_BYTE(x)	((uint8_t)(x)&0xFF)
#define HIGH_BYTE(x)	((uint8_t)(x>>8)&0xFF)
uint8_t i2c_pca9685_reset()
{
	uint8_t ret=1;
	/*ALL CALL Address = 0b0000 0000*/
	if(i2c_master_select(0x00, TW_WRITE))
	{
		TWDR = 0x06; //magic reset byte
		if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
		{
			ret=0;
		}
	}
	i2c_master_stop();
	return ret;
}
uint8_t i2c_pca9685_set_mode(uint8_t address,uint8_t outdrv,uint8_t ivrt,uint8_t prescaler)
{
	uint8_t ret0=1,ret1=1,ret2=1,ret3=1,value=0;
	ret0=i2c_pca9685_reset();
	if(ret0 == 0)
	{
		if(i2c_master_select(address, TW_WRITE))
		{
			TWDR = PRE_SCALE; //Address of Prescale Register
			if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
			{	
				value=prescaler; //Set the data to be transmitted
				TWDR=value;
#ifdef DEBUG_I2C
				debug_printf("I2C: PCA9685 writing to register %x:%d\n", PRE_SCALE, value);
#endif
				if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
				{
					ret1=0;
				}
			}

		}
		i2c_master_stop();
		if(i2c_master_select(address, TW_WRITE))
		{
			TWDR = MODE2; //Address of Mode Register 2
			if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
			{	
				value=((outdrv << 2) | (ivrt << 4)); //Set the data to be transmitted
				TWDR=value;
#ifdef DEBUG_I2C
				debug_printf("I2C: PCA9685 writing to register %x:%d\n", MODE2, value);
#endif
				if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
				{
					ret2=0;
				}
			}
		}
		i2c_master_stop();
		if(i2c_master_select(address, TW_WRITE))
		{		
			TWDR = MODE1; //Address of Mode Register 1
			if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
			{
				/*The chip is in sleep mode on power up..let's wake it up
				  Settings are: Auto-Increment: true ALLCALL: true
				 */
				value=0b00100001;
				TWDR=value;
#ifdef DEBUG_I2C
				debug_printf("I2C: PCA9685 writing to register %x:%d\n", MODE1,value);
#endif
				if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
				{
					ret3=0;
				}
			}
		}
		i2c_master_stop();
	}
	return ret0|ret1|ret2|ret3; //0 if everything went fine, 1 if one transmit failed
}
/* Setting values:
   The PCA9685 not only allows you to set a value where the output changes from HI to LOW but you can also control when
   the output changes from LOW to HI

   HI  |   ___________________          |   __________________          |
   |   |                 |          |   |                |          |
   LOW |___|                 |__________|___|                |__________|
   |LED_ON           | LED OFF      | LED_ON         | LED_OFF

i2c_pca9685_set_led:
off/on can be a value from 0-4096 (4096 means ALWAYS_ON/ALWAYS_OFF)
 */
uint8_t i2c_pca9685_set_led(uint8_t address,uint8_t led,uint16_t on, uint16_t off)
{
	uint8_t ret1=1,value=0;
#ifdef DEBUG_I2C
	debug_printf("I2C: PCA9685 writing to chip %x setting led %d to %d,%d\n", address,led,on,off);
#endif
	if(i2c_master_select(address, TW_WRITE))
	{
		value=LED0_ON_L+4*led-1; //Address of LED REGISTER low byte of the word
		TWDR=value;
		if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
		{
			value = LOW_BYTE(on); //get lower byte
			TWDR=value;
			if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
			{
				value = HIGH_BYTE(on); //get higher byte
				TWDR=value;
				if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
				{
					value = LOW_BYTE(off); //get lower byte
					TWDR=value;
					if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
					{
						value = HIGH_BYTE(off); //get higher byte
						TWDR=value;
						if(i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
						{
							ret1=0;
						}
					}
				}
			}
		}
	}
	i2c_master_stop();
	return ret1; //0 if everything went fine, 1 if one transmit failed
}

#endif
