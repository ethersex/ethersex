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
#include "../debug.h"

#ifdef HD44780_SUPPORT

/* global variables */
FILE *lcd;
uint8_t current_pos = 0;


/* macros for defining the data pins as input or output */
#ifdef HD44780_RW
  #define CTRLMASK (_BV(HD44780_RS) | _BV(HD44780_RW) | _BV(HD44780_EN))
#else
  #define CTRLMASK (_BV(HD44780_RS) | _BV(HD44780_EN))
#endif

#define DATAMASK (_BV(HD44780_D4) | _BV(HD44780_D5) | _BV(HD44780_D6) | _BV(HD44780_D7))

#define CONCAT(a,b)     a##b
#define CONCAT_PIN(x)   CONCAT(PIN,x)
#define CONCAT_DDR(x)   CONCAT(DDR,x)
#define CONCAT_PORT(x)  CONCAT(PORT,x)

#define DATA_PORT       CONCAT_PORT(HD44780_DATA_PORT)
#define DATA_DDR        CONCAT_DDR(HD44780_DATA_PORT)
#define DATA_PIN        CONCAT_PIN(HD44780_DATA_PORT)
#define CTRL_PORT       CONCAT_PORT(HD44780_CTRL_PORT)
#define CTRL_DDR        CONCAT_DDR(HD44780_CTRL_PORT)

#define DATA_INPUT() do {                               \
                            DATA_PORT |= DATAMASK;     \
                            DATA_DDR &= ~DATAMASK;      \
                        } while (0)
#define DATA_OUTPUT() DATA_DDR |= DATAMASK
#define CTRL_OUTPUT() CTRL_DDR |= CTRLMASK

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
static noinline void delay(uint8_t ms);
static noinline uint8_t clock_rw(uint8_t read);
#define clock_write() clock_rw(0)
#define clock_read() clock_rw(1)
static noinline void output_nibble(uint8_t rs, uint8_t nibble);
static noinline void output_byte(uint8_t rs, uint8_t data);
#ifdef HD44780_READBACK
static noinline uint8_t input_nibble(uint8_t rs);
static noinline uint8_t input_byte(uint8_t rs);
#endif


/* busy-loop for ms milliseconds */
void delay(uint8_t ms)
{
    for (uint8_t i = 0; i < ms; i++)
        _delay_loop_2(F_CPU/4000);
}

uint8_t noinline clock_rw(uint8_t read)
{

    /* set EN high, wait for more than 450 ns */
    CTRL_PORT |= _BV(HD44780_EN);

    /* make sure that we really wait for more than 450 ns... */
    _delay_us(0.6);

    /* read data, if requested.  data pins must be configured as input! */
    uint8_t data = 0;
    if (read) {
        data = DATA_PIN & DATAMASK;
        data >>= HD44780_DATA_SHIFT;
    }

    /* set EN low */
    CTRL_PORT &= ~_BV(HD44780_EN);

    return data;
}

void output_nibble(uint8_t rs, uint8_t nibble)
{
    /* switch to write operation and set rs */
#ifdef HD44780_RW
    CTRL_PORT &= ~_BV(HD44780_RW);
#endif
    if (rs)
        CTRL_PORT |= _BV(HD44780_RS);
    else
        CTRL_PORT &= ~_BV(HD44780_RS);

    /* compute data bits */
    uint8_t data = 0;
    if (nibble & _BV(0))
        data |= _BV(HD44780_D4);
    if (nibble & _BV(1))
        data |= _BV(HD44780_D5);
    if (nibble & _BV(2))
        data |= _BV(HD44780_D6);
    if (nibble & _BV(3))
        data |= _BV(HD44780_D7);

    /* set bits in mask, and delete bits not in mask */
    DATA_OUTPUT();
    uint8_t old_value = DATA_PORT;
    DATA_PORT = (old_value & ~DATAMASK) | data;

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
    CTRL_PORT |= _BV(HD44780_RW);

    /* set rs, if given */
    if (rs)
        CTRL_PORT |= _BV(HD44780_RS);
    else
        CTRL_PORT &= ~_BV(HD44780_RS);

    uint8_t data = clock_read();

    /* reconfigure data pins as output */
    DATA_OUTPUT();

    /* delete RW bit */
    CTRL_PORT &= ~_BV(HD44780_RW);

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

void hd44780_init(uint8_t cursor, uint8_t blink)
{

    /* init io pins */
    CTRL_OUTPUT();
    CTRL_PORT &= ~CTRLMASK;
    DATA_OUTPUT();
    DATA_PORT &= ~DATAMASK;

    delay(40);
    DATA_PORT = _BV(HD44780_D4) | _BV(HD44780_D5);
    clock_write();

    delay(4);
    DATA_PORT = _BV(HD44780_D4) | _BV(HD44780_D5);
    clock_write();

    delay(1);
    DATA_PORT = _BV(HD44780_D4) | _BV(HD44780_D5);
    clock_write();

    /* init done */
    delay(1);
    DATA_PORT = _BV(HD44780_D5);
    clock_write();
    delay(1);

    /* configure for 4 bit, 2 lines, 5x9 font (datasheet, page 24) */
    output_byte(0, CMD_FUNCTIONSET(0, 1, 0));

    /* turn on display, cursor and blinking */
    output_byte(0, CMD_POWER(1, cursor, blink));

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
    delay(50);
#endif

    output_byte(1, d);
    current_pos++;

    if (current_pos == 80)
        current_pos = 0;

    return 0;
}

#endif
