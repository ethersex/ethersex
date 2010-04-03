/* 
 * Copyright(C) 2010 Mirko Taschenberger <mirkiway at gmx.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */


#include <util/delay.h>
#include "hardware/adc/ads7822/ads7822.h"

#ifdef ADS7822_SUPPORT

#define DATA_INPUT()	PIN_CLEAR(ADS7822_D); \
			DDR_CONFIG_IN(ADS7822_D);

#define CTRL_OUTPUT() do { \
                          DDR_CONFIG_OUT(ADS7822_CS); \
                          DDR_CONFIG_OUT(ADS7822_CLK); \
                         } while (0);

/* 
 * liest den adc Wert ein und gibt ihn zurueck
 */
uint16_t get_ads()
{
	uint16_t data = 0;

	CTRL_OUTPUT();
	DATA_INPUT();
	//Start new conversion Cycle
	PIN_SET(ADS7822_CS);
	_delay_us(2);
	PIN_CLEAR(ADS7822_CLK);
	PIN_CLEAR(ADS7822_CS);	// enable ADS7822
	_delay_us(ADS_DELAY_US);

	for (int c = 0; c < 15; c++) {

		PIN_SET(ADS7822_CLK);
		_delay_us(ADS_DELAY_US);

		if (PIN_HIGH(ADS7822_D)) data |= _BV(0);

		PIN_CLEAR(ADS7822_CLK);
		_delay_us(ADS_DELAY_US);

		data = data<<1;
	}

	PIN_SET(ADS7822_CS);	// disable ADS7822
	return (data>>1)&0x0FFF;
}

#endif
