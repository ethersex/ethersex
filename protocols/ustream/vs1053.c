/*
 * Copyright (C) 2009 by Sylwester Sosnowski <esc@ext.no-route.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/
#include <avr/io.h>
#include "vs1053.h"
#include "core/spi.h"

void cs_low()
{
	PIN_CLEAR(VS1053_CS);
}
 
void cs_high()
{
	PIN_SET(VS1053_CS);
}

int sci_read(char addr)
{
	unsigned int temp;

	cs_low();

	spi_send(0x03);		// read command
	spi_send(addr);		// address

	temp = spi_send(0x00);	// dummy to get out data the msb's
	temp <<= 8;		// shift

	temp += spi_send(0x00);	// get the lsb

	cs_high();

	return temp;
}

void sci_write(char addr, int data)
{
	cs_low();

	spi_send(0x02);		// write command
	spi_send(addr);		// address

	spi_send((data >> 8) & 0xFF);	// send first 8 msb's of data
	spi_send(data & 0xFF);		// send the lsb's

	cs_high();

}

void vs1053_sinetest(char pitch)
{
	cs_high();
	
	spi_send(0x53);
	spi_send(0xEF);
	spi_send(0x6E);
	spi_send(pitch);

	spi_send(0x00);
	spi_send(0x00);
	spi_send(0x00);
	spi_send(0x00);

	cs_low();
}	
