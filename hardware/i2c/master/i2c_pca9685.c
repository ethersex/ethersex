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
#include <avr/io.h>
#include <util/twi.h>
#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_pca9685.h"
#ifdef I2C_PCA9685_SUPPORT

/* The PCA9685 features two modes of operation: totem-pole and open-drain. Please refer to the datasheet to find out
    which one suits your setup. (Page 28, Table 11)
  | ---------------------------------------------------------------- |
  | option  | v | impact                                             |
  | ---------------------------------------------------------------- |
  | outdrv  | 0 | open-drain                                         |  					
  | outdrv  | 1 | totem-pole                                         |
  | ivrt    | 0 | normal pwm output   (LED ON means PIN is high)     |
  | ivrt    | 1 | inverted pwm output (LED ON means PIN is low)      |
  | ---------------------------------------------------------------- |
 */
uint8_t
i2c_pca9685_reset(void)
{
  uint8_t ret = 1;
  /* ALL CALL Address = 0b0000 0000 */
  if (i2c_master_select(0x00, TW_WRITE))
    goto stop;
  TWDR = 0x06;                  /* magic reset byte */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  ret = 0;
stop:
  i2c_master_stop();
  return ret;
}

uint8_t
i2c_pca9685_set_mode(uint8_t address, uint8_t outdrv, uint8_t ivrt,
                     uint8_t prescaler)
{
#ifdef PCA9685_OUTPUT_ENABLE
  DDR_CONFIG_OUT(PCA9685_OE);
#endif
  uint8_t ret = 1, value = 0;
  if (!i2c_pca9685_reset())
    goto exit;
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  TWDR = PRE_SCALE;             /* Address of Prescale Register */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  value = prescaler;            /* Set the data to be transmitted */
  TWDR = value;
#ifdef DEBUG_I2C
  debug_printf("I2C: PCA9685 writing to register %#x:%#x\n", PRE_SCALE,
               value);
#endif
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;

  i2c_master_stop();
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  TWDR = MODE2;                 /* Address of Mode Register 2 */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  value = ((outdrv << 2) | (ivrt << 4));        /* Set the data to be transmitted */
  TWDR = value;
#ifdef DEBUG_I2C
  debug_printf("I2C: PCA9685 writing to register %#x:%#x\n", MODE2, value);
#endif
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  i2c_master_stop();
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  TWDR = MODE1;                 /* Address of Mode Register 1 */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  /* The chip is in sleep mode on power up..let's wake it up
   * Settings are: Auto-Increment: true ALLCALL: true */
  value = 0x21;
  TWDR = value;
#ifdef DEBUG_I2C
  debug_printf("I2C: PCA9685 writing to register %#x:%#x\n", MODE1, value);
#endif
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  ret = 0;
stop:
  i2c_master_stop();
exit:
#ifdef DEBUG_I2C
  if (ret)
    debug_printf("I2C: PCA9685 set mode failed\n");
  else
    debug_printf("I2C: PCA9685 set mode was successful\n");
#endif
  return ret;                   /* 0 if everything went fine, 1 if one transmit failed */
}

/* 
   Setting values:
   The PCA9685 not only allows you to set a value where the output changes from HI to LOW but you can also control when
   the output changes from LOW to HI

   HI  |   ___________________          |   __________________          |
       |   |                 |          |   |                |          |
   LOW |___|                 |__________|___|                |__________|
           |LED_ON           | LED OFF      | LED_ON         | LED_OFF

i2c_pca9685_set_led:
off/on can be a value from 0-4096 (4096 means ALWAYS_ON/ALWAYS_OFF)
 */
uint8_t
i2c_pca9685_set_led(uint8_t address, uint8_t led, uint16_t on, uint16_t off)
{
  uint8_t ret = 1, value = 0;
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  value = LED0_ON_L + 4 * led - 1;      /* Address of LED REGISTER low byte of the word */
  TWDR = value;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) /* Transmit address of first register (LEDn_ON_L) */
    goto stop;
  value = LO8(on);
  TWDR = value;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) /* Transmit LEDn_ON_L */
    goto stop;
  value = HI8(on);
  TWDR = value;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) /* Transmit LEDn_ON_H */
    goto stop;
  value = LO8(off);
  TWDR = value;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) /* Transmit LEDn_OFF_L */
    goto stop;
  value = HI8(off);
  TWDR = value;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) /* Transmit LEDn_OFF_H */
    goto stop;
  ret = 0;
stop:
#ifdef DEBUG_I2C
  if (ret)
    debug_printf
      ("I2C: PCA9685 writing to chip %#x setting led %d to %d,%d failed\n",
       address, led, on, off);
  else
    debug_printf
      ("I2C: PCA9685 writing to chip %#x setting led %d to %d,%d was successful\n",
       address, led, on, off);
#endif
  i2c_master_stop();
  return ret;                   /* 0 if everything went fine, 1 if one transmit failed */
}

/*
   This function sets n (count) channels in sequence. A typical example is a RGB setup and you
   want to set one LED (R-G-B) at once. This will reduce the total transmissions compared to the
   one at a time approach by about 30% (21 transmissions / 15 transmissions) 

   the array values has the format

   values[n]=LED_ON;
   values[n+1]=LED_OFF
 */
uint8_t
i2c_pca9685_set_leds(uint8_t address, uint8_t startled, uint8_t count,
                     uint16_t * values)
{
  uint8_t ret = 1;
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  TWDR = LED0_ON_L + 4 * startled;  /* Address of LED REGISTER */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  for (uint8_t i = 0; i < count; i++)   /* Now transmit all values in values in sequence */
  {
    TWDR = LO8(values[i]);
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
      goto stop;
    TWDR = HI8(values[i]);
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
      goto stop;
  }
  ret = 0;
stop:
  i2c_master_stop();
  return ret;
}

/* 
   This function only sets the high values - this will save memory and processing time where the function gets called from
   There are two special cases that will be treated differently:
   - LED is always ON, no PWM needed: value is 4096, 'always on' bit will be set *
   - LED is always OFF, no PWM needed: value is 0, 'always off' bit will be set *

   The 'always on' bit is the 4th bit of the LEDn_ON_HI byte (see datasheet page 23)
   The 'always off' bit is the 4th bit of the LEDn_OFF_HI byte (see datasheet page 23)
 */
uint8_t
i2c_pca9685_set_leds_fast(uint8_t address, uint8_t startled, uint8_t count,
                          uint16_t * values)
{
  uint8_t ret = 1;
  if (!i2c_master_select(address, TW_WRITE))
    goto stop;
  TWDR = LED0_ON_L + 4 * startled;  /* Address of LED REGISTER */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  for (uint8_t i = 0; i < count; i++)   /* Now transmit all values in values in sequence */
  {
    TWDR = 0x00;
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)       /* transmit low byte of LEDn_ON */
      goto stop;
    if (values[i] == 4096)
      TWDR = 0x10;              /* the 4th bit of LEDn_ON_HI will be set, LED is always ON */
    else
      TWDR = 0x00;
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)       /* transmit high byte of LEDn_ON */
      goto stop;
    if (values[i] == 0 || values[i] == 4096)
      TWDR = 0x00;
    else
      TWDR = LO8(values[i]);
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)       /* transmit low byte of LEDn_OFF */
      goto stop;
    if (values[i] == 0)
      TWDR = 0x10;              /* the 4th bit of LEDn_OFF_HI will be set, LED is always OFF */
    else if (values[i] == 4096)
      TWDR = 0x00;
    else
      TWDR = HI8(values[i]);
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)       /* transmit high byte of LEDn_OFF */
      goto stop;
  }
  ret = 0;
stop:
  i2c_master_stop();
  return ret;
}

#ifdef PCA9685_OUTPUT_ENABLE
void
i2c_pca9685_output_enable(enum i2c_pca9685_output_enable_state choice)
{
  if (choice == ON)             /* PIN goes low */
    PIN_CLEAR(PCA9685_OE);
  else if (choice == OFF)       /* PIN goes high */
    PIN_SET(PCA9685_OE);
  else if (choice == TOGGLE)    /* Toggle PIN */
    PIN_TOGGLE(PCA9685_OE);
}
#endif
#endif
