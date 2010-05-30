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

/* global variables */
FILE *lcd;
uint8_t current_pos = 0;
uint8_t back_light = 0;




/*Definition for different display types
  Add some new Displays here*/
#if HD44780_TYPE == HD44780_ORIGINAL
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x20
    #define LCD_LINE_3_ADR 0x40
    #define LCD_LINE_4_ADR 0x60
#elif HD44780_TYPE == HD44780_DISPTECH
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x10
    #define LCD_LINE_4_ADR 0x50
#elif HD44780_TYPE == HD44780_KS0067B
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x14
    #define LCD_LINE_4_ADR 0x54
#elif HD44780_TYPE == HD44780_KS0066U
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x10
    #define LCD_LINE_4_ADR 0x50
#else
#error "unknown hd44780 compatible controller type!"
#endif

#define LCD_MAX_CHAR LCD_CHAR_PER_LINE * LCD_LINES

#define BUSY_FLAG 7

#define HIGH_NIBBLE(x) ((uint8_t)((x) >> 4))
#define LOW_NIBBLE(x)  ((uint8_t)((x) & 0x0f))

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
static noinline void output_byte(uint8_t rs, uint8_t data);
#ifdef HD44780_READBACK
static noinline uint8_t input_byte(uint8_t rs);
#endif


void output_byte(uint8_t rs, uint8_t data)
{
    output_nibble(rs, HIGH_NIBBLE(data));
    output_nibble(rs, LOW_NIBBLE(data));

#ifdef HD44780_READBACK
    /* wait until command is executed by checking busy flag, with timeout */

    /* max execution time is for return home command,
     * which takes at most 1.52ms = 1520us */
    uint8_t busy, timeout = 200;
    do {
        busy = input_byte(0) & _BV(BUSY_FLAG);
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
uint8_t input_byte(uint8_t rs)
{
    return input_nibble(rs) << 4 | input_nibble(rs);
}
#endif

void hd44780_clear(void)
{
    output_byte(0, CMD_CLEAR_DISPLAY());
}

void hd44780_home(void)
{
    output_byte(0, CMD_HOME());
}

void hd44780_goto(uint8_t line, uint8_t pos)
{
    current_pos = (line * LCD_CHAR_PER_LINE + pos) % LCD_MAX_CHAR;
}

void hd44780_shift(uint8_t right)
{
    output_byte(0, CMD_SHIFT(1, right?1:0));
}

void
hd44780_define_char(uint8_t n_char, uint8_t *data)
{
    if (n_char > 7) return;
    /* set cgram pointer to char number n */
    output_byte(0, CMD_SETCRAMADR(n_char * 8));
    n_char = 0;
    while (n_char < 8) {
        /* send the data to lcd into cgram */
        output_byte(1, *(data + n_char));
        n_char++;
    }
}

void hd44780_init(void)
{
    /* verschiedene Hardware initialisieren */
    hd44780_hw_init();
    _delay_ms(40);

#if HD44780_TYPE == HD44780_KS0066U
    /* Hardware initialisiert -> Standardprozedur KS0066U Datenblatt 4bit Mode */
    output_nibble(0, 0x02);
#else
    /* Hardware initialisiert -> Standardprozedur HD44780 Datenblatt 4bit Mode */
    output_nibble(0, 0x03);

    _delay_ms(4);
    clock_write();

    _delay_ms(1);
    clock_write();

    _delay_ms(1);
    output_nibble(0, 0x02);		//4bit mode
    _delay_ms(1);
    /* init done */
#endif

    /* configure for 4 bit, 2 lines, 5x9 font (datasheet, page 24) */
    output_byte(0, CMD_FUNCTIONSET(0, 1, 0));

    /* turn on display, cursor and blinking */
    hd44780_config(0,0);

    /* clear display */
    hd44780_clear();

    /* set shift and increment */
    output_byte(0, CMD_ENTRY_MODE(1, 0));

    /* set ddram address */
    output_byte(0, CMD_SETDRAMADR(0));

    /* open file descriptor */
    lcd = fdevopen(hd44780_put, NULL);

    /* set current virtual postion */
    current_pos = 0;
}

void hd44780_config(uint8_t cursor, uint8_t blink) 
{
    output_byte(0, CMD_POWER(1, cursor, blink));
}

int hd44780_put(char d, FILE *stream)
{
    uint8_t start = 0;

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
        start = LCD_LINE_1_ADR - 0 + current_pos;
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 2) - 1 )
        start = LCD_LINE_2_ADR - LCD_CHAR_PER_LINE + current_pos;
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 3) - 1 )
        start = LCD_LINE_3_ADR - (LCD_CHAR_PER_LINE * 2) + current_pos;
    else if (current_pos <= ( LCD_CHAR_PER_LINE * 4) - 1 )
        start = LCD_LINE_4_ADR - (LCD_CHAR_PER_LINE * 3) + current_pos;

    output_byte(0, CMD_SETDRAMADR(start));
#ifdef DEBUG_HD44780
    _delay_ms(50);
#endif

    output_byte(1, d);
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
