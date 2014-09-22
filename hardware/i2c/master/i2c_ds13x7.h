/*
 * Copyright (c) 2009 Dirk Tostmann <tostmann@busware.de>
 * Copyright (c) 2010 Thomas Kaiser
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

#ifndef __I2C_DS13X7_H
#define __I2C_DS13X7_H

#define I2C_SLA_DS13X7 0x68

struct ds13x7_reg
{
  uint8_t sec:7;
  uint8_t ch:1;
  uint8_t min;
  uint8_t hour;
  uint8_t day;
  uint8_t date;
  uint8_t month:7;
  uint8_t century:1;
  uint8_t year;
} __attribute__ ((__packed__));

typedef struct ds13x7_reg ds13x7_reg_t;

uint16_t i2c_ds13x7_set(uint8_t reg, uint8_t data);
uint16_t i2c_ds13x7_get(uint8_t reg);
uint8_t i2c_ds13x7_set_block(uint8_t addr, char *data, uint8_t len);
uint8_t i2c_ds13x7_get_block(uint8_t addr, char *data, uint8_t len);
void i2c_ds13x7_sync(uint32_t timestamp);
uint32_t i2c_ds13x7_read(void);
void i2c_ds13x7_init(void);

#endif /* __I2C_ds13x7_H */
