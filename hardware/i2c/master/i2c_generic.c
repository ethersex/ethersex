/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2012 by Nicolas Kaufmann <mail-brainhunter@nota-lan.de>
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
#include "i2c_generic.h"

#ifdef DEBUG_I2C
#define DEBUGGI2C(fnc, msg...) debug_printf("I2C: %s: ", fnc); debug_printf(msg)
#else
#define DEBUGGI2C(fnc, msg...)
#endif

uint8_t
i2c_read_byte(const uint8_t chipaddress)
{
  uint8_t ret = 0xff;

  DEBUGGI2C("read_byte", "addr 0x%X (%d)\n", chipaddress, chipaddress);

  /*select slave with chipaddress */
  if (!i2c_master_select(chipaddress, TW_READ))
    goto end;
  /*read one byte */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;

  ret = TWDR;

end:
  i2c_master_stop();

  DEBUGGI2C("read_byte", "ret: 0x%X (%d)\n", ret, ret);

  return ret;
}

uint8_t
i2c_read_byte_data(const uint8_t chipaddress, const uint8_t dataaddress)
{
  uint8_t ret = 0xff;

  DEBUGGI2C("read_byte_data", "addr 0x%X (%d) daddr 0x%X (%d)\n",
            chipaddress, chipaddress, dataaddress, dataaddress);
  /*select slave in write mode */
  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;
  /*send the dataaddress */
  TWDR = dataaddress;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;
  /*select the slave in read mode */
  TWDR = (chipaddress << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

  /*get one byte from the slave */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;

  ret = TWDR;

end:
  i2c_master_stop();

  DEBUGGI2C("read_byte_data", "ret: 0x%X (%d)\n", ret, ret);

  return ret;
}

uint16_t
i2c_read_word_data(const uint8_t chipaddress, const uint8_t dataaddress)
{
  uint8_t data[2];
  uint16_t ret = 0xffff;

  DEBUGGI2C("read_word_data", "addr 0x%X (%d) daddr 0x%X (%d)\n", chipaddress,
            dataaddress);
  /*select slave in write mode */
  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;
  /*send the dataaddress */
  TWDR = dataaddress;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                // for slow devices
#endif

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;
  /*select the slave in read mode */
  TWDR = (chipaddress << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                // for slow devices
#endif

  /*get the first byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[0] = TWDR;

  DEBUGGI2C("read_word_data", "data0: 0x%X (%d)\n", data[0], data[0]);

#ifdef I2C_GENERIC_DELAYS
  _delay_ms(10);                // for slow devices
#endif

  /*get the second byte from the slave */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;
  data[1] = TWDR;

  DEBUGGI2C("read_word_data", "data1: 0x%X (%d)\n", data[1], data[1]);

  ret = data[0] << 8 | data[1];

end:
  i2c_master_stop();

  DEBUGGI2C("read_word_data", "ret: 0x%X (%d)\n", ret, ret);

  return ret;
}

uint16_t
i2c_write_byte(const uint8_t chipaddress, const uint8_t data)
{
  uint16_t ret = 0xffff;

  DEBUGGI2C("write_byte", "addr 0x%X (%d) data: 0x%X (%d)\n",
            chipaddress, chipaddress, data, data);

  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;

  TWDR = data;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  ret = data;

end:
  i2c_master_stop();
  return ret;
}

uint16_t
i2c_write_byte_data(const uint8_t chipaddress, const uint8_t dataaddress,
                    const uint8_t data)
{
  uint16_t ret = 0xffff;

  DEBUGGI2C("write_byte_data",
            "addr 0x%X (%d) daddr 0x%X (%d) data 0x%X (%d)\n", chipaddress,
            chipaddress, dataaddress, dataaddress, data, data);

  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;

  TWDR = dataaddress;

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  TWDR = data;

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  ret = data;

end:
  i2c_master_stop();
  return ret;
}

uint16_t
i2c_write_word_data(const uint8_t chipaddress, const uint8_t dataaddress,
                    const uint16_t data)
{
  uint16_t ret = 0xffff;

  DEBUGGI2C("write_word_data",
            "addr 0x%X (%d) daddr 0x%X (%d) data 0x%X (%d)\n", chipaddress,
            chipaddress, dataaddress, dataaddress, data, data);

  if (!i2c_master_select(chipaddress, TW_WRITE))
    goto end;

  TWDR = dataaddress;

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  TWDR = HI8(data);

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  TWDR = LO8(data);

  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  ret = data;

end:
  i2c_master_stop();
  return ret;
}
