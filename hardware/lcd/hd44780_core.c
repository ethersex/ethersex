/*
 * hd44780 driver library
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include <util/delay.h>

#include "hd44780.h"
#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"

/* global variables */
FILE lcd = FDEV_SETUP_STREAM (hd44780_put, NULL, _FDEV_SETUP_WRITE);
uint8_t current_pos = 0;
#ifdef HD44780_BACKLIGHT_SUPPORT
uint8_t back_light = 0;
#endif



#define LCD_MAX_CHAR LCD_CHAR_PER_LINE * LCD_LINES

#define BUSY_FLAG 7

/* display commands */
#define CMD_CLEAR_DISPLAY()     _BV(0)
#define CMD_HOME()              _BV(1)
#define CMD_ENTRY_MODE(dir, shift) \
    (_BV(2) | (shift) << 0 | (dir) << 1)
#define CMD_POWER(display, cursor, blink) \
    (_BV(3) | (display) << 2 | (cursor) << 1 | (blink) << 0)
#define CMD_SHIFT(cursor, dir) \
    (_BV(4) | (cursor) << 3 | (dir) << 2)
#define CMD_FUNCTIONSET(datalength, lines, font) \
    (_BV(5) | (datalength) << 4 | (lines) << 3 | (font) << 2)
#define CMD_SETCRAMADR(addr)    (_BV(6) | (addr))
#define CMD_SETDRAMADR(addr)    (_BV(7) | (addr))

/* own prototypes */
static noinline void output_byte(uint8_t rs, uint8_t data, uint8_t en);
#ifdef HD44780_READBACK
static noinline uint8_t input_byte(uint8_t rs, uint8_t en);
#endif


void output_byte(uint8_t rs, uint8_t data, uint8_t en)
{
    output_nibble(rs, HI4(data), en);
    output_nibble(rs, LO4(data), en);

#ifdef HD44780_READBACK
    /* wait until command is executed by checking busy flag, with timeout */

    /* max execution time is for return home command,
     * which takes at most 1.52ms = 1520us */
    uint8_t busy, timeout = 200;
    do {
        busy = input_byte(0,en) & _BV(BUSY_FLAG);
        _delay_us(10);
        timeout--;
    } while (busy && timeout > 0);
    #ifdef DEBUG
    if (timeout == 0)
        debug_printf("lcd timeout!\n");
    #endif
#else
    /* just wait the maximal time a command can take... */
    _delay_ms(2);
#endif
}

#ifdef HD44780_READBACK
uint8_t input_byte(uint8_t rs, uint8_t en)
{
    return input_nibble(rs, en) << 4 | input_nibble(rs, en);
}
#endif

void hd44780_clear()
{
    output_byte(0, CMD_CLEAR_DISPLAY(), 1);

#ifdef HD44780_MULTIENSUPPORT
    output_byte(0, CMD_CLEAR_DISPLAY(), 2);
#endif
}

void hd44780_home(void)
{
    output_byte(0, CMD_HOME(),1);
}

void hd44780_goto(uint8_t line, uint8_t pos)
{
    current_pos = ((line * LCD_CHAR_PER_LINE) + pos);// % LCD_MAX_CHAR;
}

void hd44780_shift(uint8_t right, uint8_t en)
{
    output_byte(0, CMD_SHIFT(1, right?1:0), en);
}

void
hd44780_define_char(uint8_t n_char, uint8_t *data, uint8_t en)
{
    if (n_char > 7) return;
    /* set cgram pointer to char number n */
    output_byte(0, CMD_SETCRAMADR(n_char * 8), en);
    n_char = 0;
    while (n_char < 8) {
        /* send the data to lcd into cgram */
        output_byte(1, *(data + n_char), en);
        n_char++;
    }
}

void hd44780_init(void)
{
    /* verschiedene Hardware initialisieren */
    hd44780_hw_init();
    _delay_ms(40);

#if CONF_HD44780_TYPE == HD44780_KS0066U
    /* Hardware initialisiert -> Standardprozedur KS0066U Datenblatt 4bit Mode */
    output_nibble(0, 0x02, 1);
#else
    /* Hardware initialisiert -> Standardprozedur HD44780 Datenblatt 4bit Mode */
    output_nibble(0, 0x03,1);

    _delay_ms(4);
#ifdef HD44780_2WIRE
    output_nibble(0, 0x03,1);
#else
    clock_write(1);
#endif /*HD44780_2WIRE*/

    _delay_ms(1);
#ifdef HD44780_2WIRE
	output_nibble(0, 0x03,1);
#else
    clock_write(1);
#endif /*HD44780_2WIRE*/

    _delay_ms(1);
    output_nibble(0, 0x02,1);		//4bit mode
    _delay_ms(1);
    /* init done */
#endif /*CONF_HD44780_TYPE*/

    /* configure for 4 bit, 2 lines, 5x8 font (datasheet, page 24) */
    output_byte(0, CMD_FUNCTIONSET(0, 1, 0), 1);

    /* turn on display, cursor and blinking */
    hd44780_config(0,0,1);

    /* clear display */
    hd44780_clear(1);
    /* set shift and increment */
    output_byte(0, CMD_ENTRY_MODE(1, 0),1);

    /* set ddram address */
    output_byte(0, CMD_SETDRAMADR(0),1);

/*Configure Controller 2 */
#ifdef HD44780_MULTIENSUPPORT
    /* verschiedene Hardware initialisieren */
    hd44780_hw_init();
    _delay_ms(40);

#if CONF_HD44780_TYPE == HD44780_KS0066U
    /* Hardware initialisiert -> Standardprozedur KS0066U Datenblatt 4bit Mode */
    output_nibble(0, 0x02, 2);
#else
    /* Hardware initialisiert -> Standardprozedur HD44780 Datenblatt 4bit Mode */
    output_nibble(0, 0x03,2);

    _delay_ms(4);
    clock_write(2);

    _delay_ms(1);
    clock_write(2);

    _delay_ms(1);
    output_nibble(0, 0x02,2);		//4bit mode
    _delay_ms(1);
    /* init done */
#endif

    /* configure for 4 bit, 2 lines, 5x9 font (datasheet, page 24) */
    output_byte(0, CMD_FUNCTIONSET(0, 1, 0), 2);

    /* turn on display, cursor and blinking */
    hd44780_config(0,0,2);

    /* clear display */
    hd44780_clear(2);
    /* set shift and increment */
    output_byte(0, CMD_ENTRY_MODE(1, 0),2);

    /* set ddram address */
    output_byte(0, CMD_SETDRAMADR(0),2);
#endif

    /* set current virtual postion */
    current_pos = 0;


}

void hd44780_config(uint8_t cursor, uint8_t blink, uint8_t en) 
{
    output_byte(0, CMD_POWER(1, cursor, blink),en);
}

int hd44780_put(char d, FILE *stream)
{
    uint8_t start = 0;

#ifdef HD44780_MULTIENSUPPORT
    uint8_t en = 1;
#endif
    if (d == '\n') {
	while (current_pos % LCD_CHAR_PER_LINE > 0)
	    hd44780_put(' ', stream);

	if (current_pos >= LCD_MAX_CHAR)
	    current_pos -= LCD_MAX_CHAR;

	return 0;
    } else if (d == '\r') {
	current_pos -= current_pos % LCD_CHAR_PER_LINE;
	return 0;
    }

    if (current_pos <= LCD_CHAR_PER_LINE - 1)
       {
	start = LCD_LINE_1_ADR - 0 + current_pos;
#ifdef HD44780_MULTIENSUPPORT
	en = LCD_LINE_1_EN;	
#endif
	}
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 2) - 1 )
    	{
	 start = LCD_LINE_2_ADR - LCD_CHAR_PER_LINE + current_pos;
#ifdef HD44780_MULTIENSUPPORT
	 en = LCD_LINE_2_EN;	
#endif
	}
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 3) - 1 )
        {
	 start = LCD_LINE_3_ADR - (LCD_CHAR_PER_LINE * 2) + current_pos;
#ifdef HD44780_MULTIENSUPPORT
	 en = LCD_LINE_3_EN;	
#endif
	}
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 4) - 1 )
        {
	 start = LCD_LINE_4_ADR - (LCD_CHAR_PER_LINE * 3) + current_pos;
#ifdef HD44780_MULTIENSUPPORT
	 en = LCD_LINE_4_EN;	
#endif
	}
    
 
#ifdef HD44780_MULTIENSUPPORT
    output_byte(0, CMD_SETDRAMADR(start), en);
#else
    output_byte(0, CMD_SETDRAMADR(start), 1);
#endif

#ifdef DEBUG_HD44780
    _delay_ms(50);
#endif

#ifdef HD44780_MULTIENSUPPORT
    output_byte(1, d, en);
#else
    output_byte(1, d, 1);
#endif
    current_pos++;

    if (current_pos == LCD_MAX_CHAR)
        current_pos = 0;

    return 0;
}

/*
  -- Ethersex META --
  header(hardware/lcd/hd44780.h)
  init(hd44780_init)
*/
