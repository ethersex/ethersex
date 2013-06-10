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

#include <avr/io.h>
#include <util/twi.h>

#include "config.h"
#include "i2c_master.h"
#include "i2c_generic.h"
#include "i2c_pca9632.h"

#ifdef I2C_PCA9632_SUPPORT
/*
Slave address of PCA9632 is 0x62h
(8-pin packages is fixed to this address and only one chip can be 
connected to the bus, 10-pin packages has two additional address 
pins for up to four chips on the bus).

MODE1 (register address 0x00h)
bit 4 (SLEEP) has to be set to 0 to start the oscillator and to be able to
set LED PWM values.
 
MODE2 (register address 0x01h) (* is default value)
bit  value  description
0    0*     unused
1    1*     unused
2    0*     output open-drain
     1      output totem pole
3    0*     change on STOP
     1      change on ACK (important for synchronizing chips)
4    0*     output NOT inverted
     1      output inverted
5    0*     group control = dimming
     1      group control = blinking
6    0*     reserved
7    0*     reserved
                     
LED_OUT_STATE (register address 0x08h) (* is default value)
bit  symbol  value
0,1  LDR0    00*
2,3  LDR1    00*
4,5  LDR2    00*
6,7  LDR3    00*
                     
LDRx = 00 LED driver is off
LDRx = 01 LED driver is on
LDRx = 10 LED driver is individually controlled by its PWM
LDRx = 11 LED driver is additionally controlled by group dimming or blinking
*/
                    
uint8_t
i2c_pca9632_reset(void)
{
  uint8_t ret = 1;
  /* SWRST Call Address = 0b0000 011 */
  if (!i2c_master_select(0x03, TW_WRITE))
    goto stop;
  TWDR = 0xA5;                  /* magic reset byte1 */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  TWDR = 0x5A;                  /* magic reset byte2 */
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto stop;
  ret = 0;
  I2CDEBUG("PCA9632_reset ok\n");
stop:
  i2c_master_stop();
  return ret;
}

uint8_t
i2c_pca9632_init(uint8_t address,
                 uint8_t mode1, uint8_t mode2, uint8_t led_out_state)
{
  uint16_t tmp;

  tmp = i2c_write_byte_data(address, 0, mode1);
  I2CDEBUG("write MODE1 register\n");
  if (tmp != mode1)
  {
    goto end;
  }

  tmp = i2c_write_byte_data(address, 1, mode2);
  I2CDEBUG("write MODE2 register\n");
  if (tmp != mode2)
  {
    goto end;
  }

  tmp = i2c_write_byte_data(address, 8, led_out_state);
  I2CDEBUG("write LEDOUT state register\n");
  if (tmp != led_out_state)
  {
    goto end;
  }

  return 0;
end:
  i2c_master_stop();
  I2CDEBUG("Error in i2c_pca9632_init\n");
  return 1;
}

uint8_t
i2c_pca9632_set_blink(uint8_t address, uint8_t grppwm, uint8_t grpfreq)
{
  uint8_t adr = 0x06;
  // GRPPWM is set in group duty cycle control register (address 06h)
  uint16_t tmp;

  I2CDEBUG("write GRPPWM and GRPFREQ to registers\n");

  tmp = i2c_write_byte_data(address, adr, grppwm);
  if (tmp != grppwm)
  {
    goto end;
  }

  adr++;                        // GRPFREQ is set in group frequency register (address 07h) 
  tmp = i2c_write_byte_data(address, adr, grpfreq);
  if (tmp != grpfreq)
  {
    goto end;
  }

  return 0;
end:
  i2c_master_stop();
  I2CDEBUG("Error in i2c_pca9632_set_blink\n");
  return 1;
}

uint8_t
i2c_pca9632_set_led(uint8_t address, uint8_t led_x, uint8_t pwm_x)
{
  uint16_t tmp;

  I2CDEBUG("write PWM to LED%d register\n", led_x);

  led_x = led_x + 2;            // PWM of LED0 is set in address 02h 
  tmp = i2c_write_byte_data(address, led_x, pwm_x);
  if (tmp != pwm_x)
  {
    goto end;
  }

  return 0;
end:
  i2c_master_stop();
  I2CDEBUG("Error in i2c_pca9632_set_led\n");
  return 1;
}

uint8_t
i2c_pca9632_read_led(uint8_t address, uint8_t led_x)
{
  uint8_t tmp;

  I2CDEBUG("read PWM of LED%d\n", led_x);

  led_x = led_x + 2;            // PWM of LED0 is set in address 02h 
  tmp = i2c_read_byte_data(address, led_x);
  return tmp;

}

#endif /* I2C_PCA9632_SUPPORT */
