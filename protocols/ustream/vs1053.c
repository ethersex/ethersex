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

	cs_high();

	spi_send(0x03);		// read command
	spi_send(addr);		// address

	temp = spi_send(0x00);	// dummy to get out data the msb's
	temp <<= 8;		// shift

	temp += spi_send(0x00);	// get the lsb

	cs_low();

	return temp;
}

void sci_write(char addr, int data)
{
	cs_high();

	spi_send(0x02);		// write command
	spi_send(addr);		// address

	spi_send((data >> 8) & 0xFF);	// send first 8 msb's of data
	spi_send(data & 0xFF);		// send the lsb's

	cs_low();

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
