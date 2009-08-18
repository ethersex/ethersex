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


/* macros for defining the data pins as input or output */

#define _DATA_INPUT(a)  PIN_CLEAR(HD44780_D ## a); \
                        DDR_CONFIG_IN(HD44780_D ## a);

#define DATA_INPUT() do { _DATA_INPUT(4); \
                          _DATA_INPUT(5); \
                          _DATA_INPUT(6); \
                          _DATA_INPUT(7); \
                        } while (0)

#define DATA_OUTPUT() do { \
                          DDR_CONFIG_OUT(HD44780_D4); \
                          DDR_CONFIG_OUT(HD44780_D5); \
                          DDR_CONFIG_OUT(HD44780_D6); \
                          DDR_CONFIG_OUT(HD44780_D7); \
                         } while (0);
#ifdef HAVE_HD4780_RW 
#define CTRL_OUTPUT() do { \
                          DDR_CONFIG_OUT(HD44780_EN); \
                          DDR_CONFIG_OUT(HD44780_RS); \
                          DDR_CONFIG_OUT(HD44780_RW); \
                         } while (0);
#else 
#define CTRL_OUTPUT() do { \
                          DDR_CONFIG_OUT(HD44780_EN); \
                          DDR_CONFIG_OUT(HD44780_RS); \
                          DDR_CONFIG_OUT(HD44780_RW); \
                         } while (0);
#endif

/* display commands */
#define HIGH_NIBBLE(x) ((uint8_t)((x) >> 4))
#define LOW_NIBBLE(x)  ((uint8_t)((x) & 0x0f))

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

#define BUSY_FLAG 7

/* other macros */
#define noinline __attribute__((noinline))

/* own prototypes */
static noinline uint8_t clock_rw(uint8_t read);
#define clock_write() clock_rw(0)
#define clock_read() clock_rw(1)
static noinline void output_nibble(uint8_t rs, uint8_t nibble);
static noinline void output_byte(uint8_t rs, uint8_t data);
#ifdef HD44780_READBACK
static noinline uint8_t input_nibble(uint8_t rs);
static noinline uint8_t input_byte(uint8_t rs);
#endif


uint8_t noinline clock_rw(uint8_t read)
{

    /* set EN high, wait for more than 450 ns */
    PIN_SET(HD44780_EN);

    /* make sure that we really wait for more than 450 ns... */
    _delay_us(0.6);

    /* read data, if requested.  data pins must be configured as input! */
    uint8_t data = 0;
    if (read) {
      if (PIN_HIGH(HD44780_D4)) data |= _BV(0);
      if (PIN_HIGH(HD44780_D5)) data |= _BV(1);
      if (PIN_HIGH(HD44780_D6)) data |= _BV(2);
      if (PIN_HIGH(HD44780_D7)) data |= _BV(3);
    }

    /* set EN low */
    PIN_CLEAR(HD44780_EN);

    return data;
}

void output_nibble(uint8_t rs, uint8_t nibble)
{
    /* switch to write operation and set rs */
#ifdef HAVE_HD44780_RW
    PIN_CLEAR(HD44780_RW);
#endif
    PIN_CLEAR(HD44780_RS);
    if (rs)
      PIN_SET(HD44780_RS);

    /* compute data bits */
    PIN_CLEAR(HD44780_D4);
    PIN_CLEAR(HD44780_D5);
    PIN_CLEAR(HD44780_D6);
    PIN_CLEAR(HD44780_D7);
    if (nibble & _BV(0))
        PIN_SET(HD44780_D4);
    if (nibble & _BV(1))
        PIN_SET(HD44780_D5);
    if (nibble & _BV(2))
        PIN_SET(HD44780_D6);
    if (nibble & _BV(3))
        PIN_SET(HD44780_D7);

    /* set bits in mask, and delete bits not in mask */
    DATA_OUTPUT();

    /* toggle EN */
    clock_write();
}

void output_byte(uint8_t rs, uint8_t data)
{
    output_nibble(rs, HIGH_NIBBLE(data));
    output_nibble(rs, LOW_NIBBLE(data));

#ifdef HD44780_READBACK
    /* wait until command is executed by checking busy flag, with timeout */

    /* max execution time is for return home command,
     * which takes at most 1.52ms = 152us */
    uint8_t busy, timeout = 200;
    do {
        busy = input_byte(0) & _BV(BUSY_FLAG);
        _delay_us(1);
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

uint8_t input_nibble(uint8_t rs)
{
    /* configure data pins as input */
    DATA_INPUT();

    /* set write bit */
    PIN_SET(HD44780_RW);

    /* set rs, if given */
    PIN_CLEAR(HD44780_RS);
    if (rs)
      PIN_SET(HD44780_RS);

    uint8_t data = clock_read();

    /* reconfigure data pins as output */
    DATA_OUTPUT();

    /* delete RW bit */
    PIN_CLEAR(HD44780_RW);

    return data;
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
    current_pos = (line * 20 + pos) % 80;
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

    /* init io pins */
    CTRL_OUTPUT();
    PIN_CLEAR(HD44780_RS);
    PIN_CLEAR(HD44780_EN);
#ifdef HAVE_HD44780_RW
    PIN_CLEAR(HD44780_RW);
#endif
    PIN_CLEAR(HD44780_D4);
    PIN_CLEAR(HD44780_D5);
    PIN_CLEAR(HD44780_D6);
    PIN_CLEAR(HD44780_D7);
    DATA_OUTPUT();

    _delay_ms(40);
    PIN_SET(HD44780_D4);
    PIN_SET(HD44780_D5);
    clock_write();

    _delay_ms(4);
    clock_write();

    _delay_ms(1);
    clock_write();

    /* init done */
    _delay_ms(1);
    PIN_CLEAR(HD44780_D4);
    clock_write();
    _delay_ms(1);

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
        while (current_pos % 20 > 0)
            hd44780_put(' ', stream);

        if (current_pos >= 80)
            current_pos -= 80;

        return 0;
    } else if (d == '\r') {
        current_pos -= current_pos % 20;
        return 0;
    }

#if defined(HD44780_ORIGINAL)
    if (current_pos <= 19)
        start = 0x00 - 00 + current_pos;
    else if (current_pos <= 39)
        start = 0x20 - 20 + current_pos;
    else if (current_pos <= 59)
        start = 0x40 - 40 + current_pos;
    else if (current_pos <= 79)
        start = 0x60 - 60 + current_pos;
#elif defined(HD44780_DISPTECH)
    if (current_pos <= 19)
        start = 0x00 - 0 + current_pos;
    else if (current_pos <= 39)
        start = 0x40 - 20 + current_pos;
    else if (current_pos <= 59)
        start = 0x10 - 40 + current_pos;
    else if (current_pos <= 79)
        start = 0x50 - 60 + current_pos;
#elif defined(HD44780_KS0067B)
    if (current_pos <= 19)
        start = 0x00 - 0 + current_pos;
    else if (current_pos <= 39)
        start = 0x40 - 20 + current_pos;
    else if (current_pos <= 59)
        start = 0x14 - 40 + current_pos;
    else if (current_pos <= 79)
        start = 0x54 - 60 + current_pos;
#else
#error "unknown hd44780 compatible controller type!"
#endif

    output_byte(0, CMD_SETDRAMADR(start));
#ifdef DEBUG_HD44780
    _delay_ms(50);
#endif

    output_byte(1, d);
    current_pos++;

    if (current_pos == 80)
        current_pos = 0;

    return 0;
}

/*
  -- Ethersex META --
  header(hardware/lcd/hd44780.h)
  init(hd44780_init)
*/
