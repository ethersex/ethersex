/*
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
 * Based on work from Frank Nitzsche beta-frank@beta-x.de www.beta-x.de
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
#include "i2c_pcf8583.h"
#include <avr/interrupt.h>
#include <string.h>

#include "services/clock/clock.h"

#include "core/bit-macros.h"
#include "core/debug.h"

#ifdef I2C_PCF8583_SUPPORT

#define  BIN2BCD(b)  (((b)%10) | (((b)/10)<<4))
#define  BCD2BIN(b)  (((b)&0xf) + ((b)>>4)*10)

#define PCF8583_MASK 0x00

#ifdef I2C_PCF8583_SYNC

#ifndef I2C_PCF8583_SYNC_PERIOD
#define I2C_PCF8583_SYNC_PERIOD 60
#endif /* I2C_PCF8583_SYNC_PERIOD */

static uint16_t sync_timer;
#endif /* I2C_PCF8583_SYNC */

uint8_t
i2c_pcf8583_set_addr(uint8_t Adr)
{
  if (!i2c_master_select(PCF8583_ADR, TW_WRITE))
  {
    return 0;
  }

  TWDR = Adr;
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    return 0;
  }

  return 1;
}

uint8_t
i2c_pcf8583_start_read(void)
{
  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
  {
    return 0;
  }

  TWDR = (PCF8583_ADR << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
  {
    return 0;
  }

  return 1;
}

uint8_t
i2c_pcf8583_set_byte(uint8_t Adr, uint8_t Val)
{
  uint8_t ret = 0;

#ifdef DEBUG_I2C
  debug_printf
    ("I2C: i2c_pcf8583_set_byte: 0x%X (%d) [0x%X (%d)] : [0x%X (%d)]\n",
     PCF8583_ADR, PCF8583_ADR, Adr, Adr, Val, Val);
#endif
  if (!i2c_pcf8583_set_addr(Adr))
  {
    goto end;
  }

  TWDR = Val;
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  ret = 1;

end:
  i2c_master_stop();
  return ret;
}

uint8_t
i2c_pcf8583_get_byte(uint8_t Adr)
{
  uint8_t ret = 0xff;

#ifdef DEBUG_I2C
  debug_printf("I2C: i2c_pcf8583_get_byte: 0x%X (%d) [0x%X (%d)]\n",
               PCF8583_ADR, PCF8583_ADR, Adr, Adr);
#endif
  if (!i2c_pcf8583_set_addr(Adr))
  {
    goto end;
  }

  /* start reading */
  if (!i2c_pcf8583_start_read())
  {
    goto end;
  }

  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    goto end;
  }
  ret = TWDR;

end:
  i2c_master_stop();

#ifdef DEBUG_I2C
  debug_printf("I2C: i2c_pcf8583_get_byte value: 0x%X (%d)\n", ret, ret);
#endif

  return ret;
}

uint8_t
i2c_pcf8583_set_word(uint8_t Adr, uint16_t Val)
{
  uint8_t ret = 0;

#ifdef DEBUG_I2C
  debug_printf
    ("I2C: i2c_pcf8583_set_word: 0x%X (%d) [0x%X (%d)] : [0x%X (%d)]\n",
     PCF8583_ADR, PCF8583_ADR, Adr, Adr, Val, Val);
#endif
  if (!i2c_pcf8583_set_addr(Adr))
  {
    goto end;
  }

  TWDR = HI8(Val);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }

  TWDR = LO8(Val);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  ret = 1;

end:
  i2c_master_stop();
  return ret;
}

uint16_t
i2c_pcf8583_get_word(uint8_t Adr)
{
  uint16_t ret = 0xffff;

#ifdef DEBUG_I2C
  debug_printf("I2C: i2c_pcf8583_get_word: 0x%X (%d) [0x%X (%d)]\n",
               PCF8583_ADR, PCF8583_ADR, Adr, Adr);
#endif
  if (!i2c_pcf8583_set_addr(Adr))
  {
    goto end;
  }

  /* start reading */
  if (!i2c_pcf8583_start_read())
  {
    goto end;
  }

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  ret = (uint16_t) TWDR << 8;

  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    ret = 0xffff;
    goto end;
  }
  ret |= (uint16_t) TWDR;

end:
  i2c_master_stop();

#ifdef DEBUG_I2C
  debug_printf("I2C: i2c_pcf8583_get_word value: 0x%X (%d)\n", ret, ret);
#endif

  return ret;
}

void
i2c_pcf8583_init(void)
{
  i2c_pcf8583_set_byte(PCF8583_CTRL_STATUS_REG, PCF8583_MASK);

#ifdef I2C_PCF8583_SYNC
  sync_timer = 0;
#endif

#ifdef CLOCK_SUPPORT
  i2c_pcf8583_sync();
#endif
}

void
i2c_pcf8583_set(uint32_t timestamp)
{
#ifdef CLOCK_DATETIME_SUPPORT
  pcf8583_reg_t dt;
  clock_datetime_t d;

  memset(&dt, 0, sizeof(dt));
  clock_localtime(&d, timestamp);

  dt.hsec = 0;
  dt.sec = d.sec;
  dt.min = d.min;
  dt.hour = d.hour;

  dt.wday = d.dow;

  dt.day = d.day;
  dt.mon = d.month;
  dt.year = d.year;

  i2c_pcf8583_set_rtc(&dt);
#endif
}

uint32_t
i2c_pcf8583_get(void)
{
#ifdef CLOCK_DATETIME_SUPPORT
  pcf8583_reg_t dt;
  clock_datetime_t d;

  if (!i2c_pcf8583_get_rtc(&dt))
    return 0;
  d.sec = dt.sec;
  d.min = dt.min;
  d.hour = dt.hour;
  d.dow = dt.wday;
  d.day = dt.day;
  d.month = dt.mon;
  d.year = dt.year;
  d.isdst = 0;

  return clock_mktime(&d, 1);
#else
  return 0;
#endif /* CLOCK_DATETIME_SUPPORT */
}

void
i2c_pcf8583_sync(void)
{
#ifdef CLOCK_DATETIME_SUPPORT
  pcf8583_reg_t dt;
  clock_datetime_t d;

  if (!i2c_pcf8583_get_rtc(&dt))
    return;
  d.sec = dt.sec;
  d.min = dt.min;
  d.hour = dt.hour;
  d.dow = dt.wday;
  d.day = dt.day;
  d.month = dt.mon;
  d.year = dt.year;
  d.isdst = 0;

  clock_set_time_raw_hr(clock_mktime(&d, 1), (dt.hsec) >> 1);
#endif /* CLOCK_DATETIME_SUPPORT */
}

uint8_t
i2c_pcf8583_reset_rtc(void)
{
  pcf8583_reg_t dt;
  dt.day = 1;
  dt.wday = 6;                  /* Saturday */
  dt.mon = 1;
  dt.year = 2000;
  dt.hour = 0;
  dt.min = 0;
  dt.sec = 0;
  dt.hsec = 0;
  return i2c_pcf8583_set_rtc(&dt);
}

uint8_t
i2c_pcf8583_set_rtc(pcf8583_reg_t * dt)
{
  uint8_t ret = 0;

#ifdef DEBUG_I2C
  debug_printf
    ("I2C: i2c_pcf8583_set_rtc: 0x%X (%d) : %04d-%02d-%02d (%d) %02d:%02d:%02d.%02d\n",
     PCF8583_ADR, PCF8583_ADR, dt->year, dt->mon, dt->day, dt->wday, dt->hour,
     dt->min, dt->sec, dt->hsec);
#endif
  if (!i2c_pcf8583_set_addr(PCF8583_CTRL_STATUS_REG))
  {
    goto end;
  }

  /* Stop counting */
  TWDR = PCF8583_STOP_COUNTING | PCF8583_MASK;
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }

  /* Time */
  TWDR = BIN2BCD(dt->hsec);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  TWDR = BIN2BCD(dt->sec);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  TWDR = BIN2BCD(dt->min);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  TWDR = BIN2BCD(dt->hour) & 0x3f;      /* 24h mode */
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }

  /* Date */
  TWDR = (BIN2BCD(dt->day) & 0x3f) | ((dt->year & 0x03) << 6);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  TWDR = (BIN2BCD(dt->mon) & 0x1f) | ((dt->wday & 0x07) << 5);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    goto end;
  }
  ret = 1;

end:
  i2c_master_stop();
  if (!ret)
    return 0;

  /* update Year register */
  if (!i2c_pcf8583_set_word(PCF8583_YEAR_REG, dt->year))
    return 0;

  /* Enable counting */
  if (!i2c_pcf8583_set_byte(PCF8583_CTRL_STATUS_REG, PCF8583_MASK))
    return 0;

  return 1;
}

uint8_t
i2c_pcf8583_get_rtc(pcf8583_reg_t * dt)
{
  uint8_t ret = 0;
  uint8_t YearCnt = 0;
  uint16_t YearReg;

#ifdef DEBUG_I2C
  debug_printf("I2C: i2c_pcf8583_get_rtc: 0x%X (%d)\n", PCF8583_ADR,
               PCF8583_ADR);
#endif
  if (!i2c_pcf8583_set_addr(PCF8583_100S_REG))
  {
    goto end;
  }

  /* start reading */
  if (!i2c_pcf8583_start_read())
  {
    goto end;
  }

  /* Time */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  dt->hsec = BCD2BIN(TWDR);
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  dt->sec = BCD2BIN(TWDR);
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  dt->min = BCD2BIN(TWDR);
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  dt->hour = BCD2BIN(TWDR & 0x3f);

  /* Date */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
  {
    goto end;
  }
  YearCnt = TWDR >> 6;
  dt->day = BCD2BIN(TWDR & 0x3f);
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    goto end;
  }
  dt->mon = BCD2BIN(TWDR & 0x1f);
  dt->wday = TWDR >> 5;
  ret = 1;

end:
  i2c_master_stop();
  if (!ret)
    return 0;

  /* Read the year from NVRAM. */
  YearReg = i2c_pcf8583_get_word(PCF8583_YEAR_REG);
  if (YearReg == 0xffff)
    return 0;

  /* Increment passed years */
  YearReg += (YearCnt - (YearReg & 0x03)) & 0x03;

  /* Now update the year byte in the NVRAM */
  if (!i2c_pcf8583_set_word(PCF8583_YEAR_REG, YearReg))
    return 0;

  dt->year = YearReg;

#ifdef DEBUG_I2C
  debug_printf
    ("I2C: i2c_pcf8583_get_rtc value: %04d-%02d-%02d (%d) %02d:%02d:%02d.%02d\n",
     dt->year, dt->mon, dt->day, dt->wday, dt->hour, dt->min, dt->sec,
     dt->hsec);
#endif

  return 1;
}

void
i2c_pcf8583_tick(void)
{
#ifdef I2C_PCF8583_SYNC
  if (clock_get_ticks() == 25)
  {
    if (++sync_timer >= I2C_PCF8583_SYNC_PERIOD)
    {
      sync_timer = 0;
      i2c_pcf8583_sync();
    }
  }
#endif
}

#endif /* I2C_PCF8583_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_pcf8583.h)
  init(i2c_pcf8583_init)
  timer(1, i2c_pcf8583_tick())
*/
