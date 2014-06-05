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

#include "config.h"
#include "autoconf.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "i2c_master.h"
#include "i2c_bh1750.h"

#ifdef DEBUG_I2C
#define DEBUGGI2C(fnc, msg...) debug_printf("I2C: %s: ", fnc); debug_printf(msg)
#else
#define DEBUGGI2C(fnc, msg...)
#endif

#define PowerDown    0x00
#define PowerOn      0x01
#define Reset        0x07
#define ContinuHigh  0x10
#define ContinuLow   0x13
#define OneTimeHigh  0x20
#define OneTimeLow   0x23

#ifdef I2C_BH1750_SUPPORT

int16_t
i2c_bh1750_read_lux(const uint8_t chipaddress)
{
  uint8_t data[2];
  uint16_t ret = 0xffff;
  uint8_t chipmode = 0x10;

  /*select slave in write mode */
  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;
  /*send the dataaddress */
  TWDR = chipmode;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  _delay_ms(10);		// for slow devices

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;
  /*select the slave in read mode */
  TWDR = (chipaddress << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;


  _delay_ms(250);                // wait for conversion


  /*get the first byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[0] = TWDR;

  DEBUGGI2C("read_word_data", "data0: 0x%X (%d)\n", data[0], data[0]);


  /*get the second byte from the slave */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;
  data[1] = TWDR;

  DEBUGGI2C("read_word_data", "data1: 0x%X (%d)\n", data[1], data[1]);

  ret = (data[0] << 8 | data[1]) / 1.2;

end:
  i2c_master_stop();

  DEBUGGI2C("read_word_data", "ret: 0x%X (%d)\n", ret, ret);

  return ret;
}

#endif /* I2C_BH1750_SUPPORT */
