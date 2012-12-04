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

#include <avr/io.h>
#include <util/twi.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_generic.h"

#include "i2c_mcp9801.h"

#ifdef I2C_MCP9801_SUPPORT

uint16_t i2c_mcp9801_set_resolution(const uint8_t chipaddress,
		const uint8_t resolution) {

	uint8_t data = ((0b00000011 & resolution) << 5); // only last two bits then shifting to right position

	return i2c_write_byte_data(chipaddress, MCP9801_REG_CONFIG, data);
}

uint16_t i2c_mcp9801_read_temperature(const uint8_t chipaddress,
		uint8_t *negative, uint16_t *tempM, uint16_t *tempL) {

	uint16_t readout = i2c_read_word_data(chipaddress, MCP9801_REG_TA);
	//                    \/
	// readout = 0b0001100101110000; // 25.4375
	// readout = 0b0000000000010000; // 0.0625
	// readout = 0b0000000000000000; // 0.0
	// readout = 0b1111111111110000; // -0.0625
	// readout = 0b1110011010010000; // -25.4375
	// readout = 0b1111111100000000; // -1.0
	// readout = 0b1111111000000000; // -2.0

	if (readout >= 0x8000) { // if first bit is set, temperature is negative
		*negative = 1;
		readout = 32768 - (readout & 0x7fff); // make it positive
	} else {
		*negative = 0;
	}

	*tempM = ((readout & 0xFF00) >> 8); // msb: first byte
	*tempL = ((readout & 0x00FF) >> 4) * 625; // lsb: 4bit not used --> shift >> 4 * (1/(2^4)*10^4)

#ifdef DEBUG_I2C
	debug_printf("I2C: i2c_mcp9801_read_temp: temp: %d.%d\n",*tempM,*tempL); // FIXME
#endif

	return 0x0;
}

#endif /* I2C_MCP9801_SUPPORT */
