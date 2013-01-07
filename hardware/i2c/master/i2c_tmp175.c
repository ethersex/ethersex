/*
 * Copyright (c) 2011 by Patrick Hieber <patrick.hieber@gmx.net>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"

#include "i2c_tmp175.h"

int16_t
i2c_tmp175_read_temp(uint8_t address)
{
  uint8_t temp[2];
  int16_t ret = 0xffff;

#ifdef DEBUG_I2C
  debug_printf("I2C::tmp175 read_word_data",
               "addr 0x%X (%d) daddr 0x%X (%d)\n", address,
               I2C_SLA_TMP175_DR);
#endif
  if (!i2c_master_select(address, TW_WRITE))
    goto end;

  TWDR = I2C_SLA_TMP175_DR;     //read the temerature register (=0x00)

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                //for slow devices
#endif

  /* Do a repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;
  TWDR = (address << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                //for slow devices
#endif

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  temp[0] = TWDR;

#ifdef DEBUG_I2C
  debug_printf("I2C::tmp175 read_word_data", "temp0: 0x%X (%d)\n", temp[0],
               temp[0]);
#endif

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                // for slow devices
#endif

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  temp[1] = TWDR;

#ifdef DEBUG_I2C
  debug_printf("I2C::tmp175 read_word_data", "temp1: 0x%X (%d)\n", temp[1],
               temp[1]);
#endif

  // @see datasheet p6/20
  // temp[0] highbyte
  // temp[1] lowbyte
  ret = 0x0000;
  ret = ((temp[0] << 8) | (temp[1] >> 4));

#ifdef DEBUG_I2C
  debug_printf("I2C::tmp175 read_word_data", "ret: 0x%X (%d)\n", ret, ret);
#endif

end:
  i2c_master_stop();
  return ret;
}
