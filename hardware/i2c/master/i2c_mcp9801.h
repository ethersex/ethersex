/*
 *
 * Copyright (c) 2012 by Daniel Schulte <daniel@schulte.me>
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

#ifndef _I2C_MCP9801_H
#define _I2C_MCP9801_H

#define I2C_SLA_MCP9801 0x48

#define MCP9801_REG_TA     0x0
#define MCP9801_REG_CONFIG 0x1
#define MCP9801_REG_THYST  0x2
#define MCP9801_REG_TSET   0x3

/*
 * 0b00 = 9 bit or 0.5°C (Power-up default)
 * 0b01 = 10 bit or 0.25°C
 * 0b10 = 11 bit or 0.125°C
 * 0b11 = 12 bit or 0.0625°C
 */
#define MCP9801_RES_9  0x0
#define MCP9801_RES_10 0x1
#define MCP9801_RES_11 0x2
#define MCP9801_RES_12 0x3

uint16_t i2c_mcp9801_set_resolution(const uint8_t chipaddress,
		const uint8_t resolution);
uint16_t i2c_mcp9801_read_temperature(const uint8_t chipaddress,
		uint8_t *negative, uint16_t *tempM, uint16_t *tempL);

#endif /* _I2C_MCP9801_H */
