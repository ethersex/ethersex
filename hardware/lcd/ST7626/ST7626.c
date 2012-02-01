/*
* Copyright (c) 2009 by Bastian Nagel <slime@voiceteam.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "ST7626.h"
#include "4x6.h"
#include "config.h"

#define COMMAND		0
#define DATA		1
#define BLACK		0x00
#define WHITE		0xFF

FILE lcd = FDEV_SETUP_STREAM(ST7626_putc, NULL, _FDEV_SETUP_WRITE);
uint16_t current_pos;

uint8_t ST7626_paint[4][16] PROGMEM = {
    { 0, 1, 4, 6, 8,11,12,13,14,16,17,18,19,21,24,28},
    { 0, 2, 5, 7, 9,11,12,13,14,16,17,18,19,22,25,29},
    { 0, 3, 6, 8,10,12,13,14,15,16,17,19,20,23,26,30},
    { 0, 4, 7, 9,11,12,13,14,15,16,17,19,21,24,27,31}
};

void ST7626_reset(void)
{
    PIN_CLEAR(ST7626_RESET);
    _delay_us(2);
    PIN_SET(ST7626_RESET);
    _delay_us(2);
}

void ST7626_write(uint8_t mode, uint8_t data)
{
    ST7626_DATA_DDR = 0xFF;
    
    if(mode)
	PIN_SET(ST7626_A0);
    else
	PIN_CLEAR(ST7626_A0);
	
    ST7626_DATA = data;
    PIN_CLEAR(ST7626_CS);
    PIN_CLEAR(ST7626_WR);
    asm("nop");
    asm("nop");
    PIN_SET(ST7626_WR);
    PIN_SET(ST7626_CS);
}

void ST7626_load_eeprom(void)
{
    ST7626_write(COMMAND, 0x31);
    ST7626_write(COMMAND, 0xcd);
    ST7626_write(DATA, 0x00);
    _delay_ms(50);
    ST7626_write(COMMAND, 0xfd);
    _delay_ms(50);
    ST7626_write(COMMAND, 0xcc);
    ST7626_write(COMMAND, 0x30);
}

void ST7626_load_paint(void)
{
    uint8_t frame, par;
    
    ST7626_write(COMMAND, 0x31);
    for(frame = 0; frame < 4; frame++)
    {
	ST7626_write(COMMAND, (0x20 + frame));
	for(par = 0; par < 16; par++)
	    ST7626_write(DATA, pgm_read_byte(&ST7626_paint[frame][par]));
    }
    ST7626_write(COMMAND, 0x30);
}

void ST7626_init(void)
{
    PIN_SET(ST7626_WR);
    PIN_SET(ST7626_RD);

    ST7626_reset();

    ST7626_write(COMMAND, 0x30);
    ST7626_write(COMMAND, 0x04);
    ST7626_write(DATA, 0x3e);

    ST7626_write(COMMAND, 0x31);
    ST7626_write(COMMAND, 0xf4);
    ST7626_write(DATA, 0x58);

    ST7626_write(COMMAND, 0x30);
    ST7626_write(COMMAND, 0x94);
    ST7626_write(COMMAND, 0xd1);
    ST7626_write(COMMAND, 0xca);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 0x10);
    ST7626_write(DATA, 0x00);

    ST7626_write(COMMAND, 0x31);
    ST7626_write(COMMAND, 0x32);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 0x02);
    ST7626_write(DATA, 0x03);
    ST7626_write(DATA, 0x04);

    ST7626_write(COMMAND, 0x30);
    ST7626_write(COMMAND, 0x81);
    ST7626_write(DATA, 0x1d);
    ST7626_write(DATA, 0x02);

    ST7626_write(COMMAND, 0x20);
    ST7626_write(DATA, 0x0b);

    _delay_ms(50);
    ST7626_load_eeprom();
    ST7626_load_paint();
    
    ST7626_write(COMMAND, 0x30);
    ST7626_write(COMMAND, 0xa7);
    ST7626_write(COMMAND, 0xbb);
    ST7626_write(DATA, 0x01);
    ST7626_write(COMMAND, 0xbc);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 0x01);

    ST7626_write(COMMAND, 0x75);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 67);
    ST7626_write(COMMAND, 0x15);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 97);

    ST7626_write(COMMAND, 0xaf);
    ST7626_write(COMMAND, 0x30);

    ST7626_clear(WHITE);
}

void ST7626_clear(uint8_t color)
{
    uint8_t x, y;

    ST7626_write(COMMAND, 0x75);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 67);
    ST7626_write(COMMAND, 0x15);
    ST7626_write(DATA, 0x00);
    ST7626_write(DATA, 97);
    ST7626_write(COMMAND, 0x5c);

    for(y = 0; y < 67; y++)
    {
	for(x = 0; x < 98; x++)
	{
	    ST7626_write(DATA, color);
	    ST7626_write(DATA, color);
	}
    }

    current_pos = 0;
}

int ST7626_putc(char d, FILE *stream)
{
    uint8_t x, y;
    
    x = (current_pos % 24) * 4;
    y = (current_pos / 24) * 6;
    
    ST7626_write(COMMAND, 0x15);
    ST7626_write(DATA, x);
    ST7626_write(DATA, (x+3));
    ST7626_write(COMMAND, 0x75);
    ST7626_write(DATA, y);
    ST7626_write(DATA, (y+5));
    ST7626_write(COMMAND, 0x5c);

    for(y = 0; y < 6; y++)
    {
	for(x = 0; x < 4; x++)
	{
	    if((pgm_read_byte(&font[(unsigned char)d][x])>>(y+1)) & 1)
	    {
		ST7626_write(DATA, 0x00);
		ST7626_write(DATA, 0x00);
	    }
	    else
	    {
		ST7626_write(DATA, 0xFF);
		ST7626_write(DATA, 0xFF);
	    }
	}
    }

    current_pos++;
        
    return 0;
}

void ST7626_putpixel(uint8_t x, uint8_t y, uint16_t color)
{
    ST7626_write(COMMAND, 0x15);
    ST7626_write(DATA, x);
    ST7626_write(DATA, (x+1));
    ST7626_write(COMMAND, 0x75);
    ST7626_write(DATA, y);
    ST7626_write(DATA, (y+1));
    ST7626_write(COMMAND, 0x5c);

    ST7626_write(DATA, (unsigned char)(color>>8));
    ST7626_write(DATA, (unsigned char)color);
}

/*
  -- Ethersex META --
  header(hardware/lcd/ST7626/ST7626.h)
  init(ST7626_init)
*/
