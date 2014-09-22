/* 
 * Copyright (c) 2009 Dirk Tostmann <tostmann@busware.de>
 * Copyright (c) 2010 Thomas Kaiser
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
#include "i2c_ds13x7.h"
#include <string.h>

#include "services/clock/clock.h"

uint8_t
i2c_ds13x7_set_block(uint8_t addr, char *data, uint8_t len)
{
  uint8_t ret = 0;

  if (!i2c_master_select(I2C_SLA_DS13X7, TW_WRITE))
  {
    ret = 1;
    goto end;
  }

  TWDR = addr;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
  {
    ret = 2;
    goto end;
  }

  for (uint8_t i = 0; i < len; i++)
  {
    TWDR = *(data + i);
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    {
      ret = 3;
      goto end;
    }
  }

end:
  i2c_master_stop();
  return ret;
}

uint8_t
i2c_ds13x7_get_block(uint8_t addr, char *data, uint8_t len)
{
  uint8_t ret = 0;

  if (!i2c_master_select(I2C_SLA_DS13X7, TW_WRITE))
  {
    ret = 1;
    goto end;
  }

  TWDR = addr;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
  {
    ret = 2;
    goto end;
  }

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
  {
    ret = 3;
    goto end;
  }

  /* Send the address again */
  TWDR = (I2C_SLA_DS13X7 << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
  {
    ret = 4;
    goto end;
  }

  len--;
  for (uint8_t i = 0; i < len; i++)
  {
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    {
      ret = 5;
      goto end;
    }
    *(data + i) = TWDR;
  }
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    ret = 6;
    goto end;
  }
  *(data + len) = TWDR;

end:
  i2c_master_stop();
  return ret;
}

uint16_t
i2c_ds13x7_set(uint8_t reg, uint8_t data)
{
  if (i2c_ds13x7_set_block(reg, (char *) &data, 1))
    return 0x100;
  return 0;
}

uint16_t
i2c_ds13x7_get(uint8_t reg)
{
  uint8_t ret = 0;

  if (i2c_ds13x7_get_block(reg, (char *) &ret, 1))
    return 0x100;

  return ret;
}

uint8_t
b2i(uint8_t bcd)
{
  return 10 * (bcd >> 4) + (bcd & 0xf);
}

uint8_t
i2b(uint8_t i)
{
  return ((i / 10) << 4) | i % 10;
}


void
i2c_ds13x7_sync(uint32_t timestamp)
{
#ifdef CLOCK_DATETIME_SUPPORT
  ds13x7_reg_t rtc;
  clock_datetime_t d;

  memset(&rtc, 0, sizeof(rtc));

  clock_localtime(&d, timestamp);

  rtc.ch = 0;
  rtc.sec = i2b(d.sec);
  rtc.min = i2b(d.min);
  rtc.hour = i2b(d.hour);

  rtc.day = d.dow;

  rtc.date = i2b(d.day);
  rtc.month = i2b(d.month);
  rtc.century = (d.year >= 100 ? 1 : 0);
  rtc.year = i2b(d.year % 100);
  i2c_ds13x7_set_block(0, (char *) &rtc, sizeof(rtc));  // not the Ctrl reg
#endif
}

uint32_t
i2c_ds13x7_read(void)
{
#ifdef CLOCK_DATETIME_SUPPORT
  ds13x7_reg_t rtc;
  clock_datetime_t d;

  i2c_ds13x7_get_block(0, (char *) &rtc, sizeof(rtc));
  d.sec = b2i(rtc.sec);
  d.min = b2i(rtc.min);
  d.hour = b2i(rtc.hour);
  d.dow = rtc.day;
  d.day = b2i(rtc.date);
  d.month = b2i(rtc.month & 0x1f);
  d.year = b2i(rtc.year);
  if (rtc.century)
    d.year += 100;
  return clock_mktime(&d, 1);
#else
  return 0;
#endif /* CLOCK_DATETIME_SUPPORT */

}

#ifdef CLOCK_SUPPORT
void
i2c_ds13x7_init(void)
{
  clock_set_time_raw(i2c_ds13x7_read());
}
#endif

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_ds13x7.h)
  ifdef(`conf_CLOCK',`init(i2c_ds13x7_init)')
*/
