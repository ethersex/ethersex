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

#ifndef _I2C_PCF8583_H
#define _I2C_PCF8583_H

#ifdef I2C_PCF8583_A0_VCC
#define PCF8583_ADR 0x51        // Slave address PCF8583
#else
#define PCF8583_ADR 0x50        // Slave address PCF8583
#endif

// Register addresses
#define PCF8583_CTRL_STATUS_REG    0x00
#define PCF8583_100S_REG           0x01
#define PCF8583_SECONDS_REG        0x02
#define PCF8583_MINUTES_REG        0x03
#define PCF8583_HOURS_REG          0x04
#define PCF8583_DATE_REG           0x05
#define PCF8583_MONTHS_REG         0x06
#define PCF8583_TIMER_REG          0x07

#define PCF8583_ALARM_CONTROL_REG  0x08
#define PCF8583_ALARM_100S_REG     0x09
#define PCF8583_ALARM_SECS_REG     0x0A
#define PCF8583_ALARM_MINS_REG     0x0B
#define PCF8583_ALARM_HOURS_REG    0x0C
#define PCF8583_ALARM_DATE_REG     0x0D
#define PCF8583_ALARM_MONTHS_REG   0x0E
#define PCF8583_ALARM_TIMER_REG    0x0F

// Use the first NVRAM address for the year byte.
#define PCF8583_YEAR_REG           0x10

// Commands for the Control/Status register.
#define PCF8583_START_COUNTING     0x00
#define PCF8583_STOP_COUNTING      0x80

// struct for holding date/time
struct pcf8583_reg
{
  uint8_t hsec;
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint8_t day;
  uint8_t wday;
  uint8_t mon;
  uint16_t year;
} __attribute__ ((__packed__));

typedef struct pcf8583_reg pcf8583_reg_t;

void i2c_pcf8583_tick(void);

void i2c_pcf8583_init(void);
void i2c_pcf8583_set(uint32_t timestamp);
uint32_t i2c_pcf8583_get(void);
void i2c_pcf8583_sync(void);

uint8_t i2c_pcf8583_set_byte(uint8_t reg, uint8_t data);
uint8_t i2c_pcf8583_get_byte(uint8_t data);
uint8_t i2c_pcf8583_set_word(uint8_t reg, uint16_t data);
uint16_t i2c_pcf8583_get_word(uint8_t data);

uint8_t i2c_pcf8583_reset_rtc(void);
uint8_t i2c_pcf8583_set_rtc(pcf8583_reg_t * dt);
uint8_t i2c_pcf8583_get_rtc(pcf8583_reg_t * dt);

#endif /* _I2C_PCF8583_H */
