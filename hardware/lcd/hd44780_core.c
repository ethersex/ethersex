/*
 * hd44780 driver library
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2016 Michael Brakemeier <michael@brakemeier.de>
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

#include "config.h"

#include "hd44780.h"

#include "core/debug.h"
#include "core/bit-macros.h"


#if defined(DEBUG_HD44780) && !defined(TEENSY_SUPPORT)
#define DEBUG_LCD(fnc, s, msg...) printf_P(PSTR("D: LCD: " #fnc ": " s), ## msg)
#else
#define DEBUG_LCD(fnc, s, msg...)
#endif

#define LCD_MAX_CHAR (LCD_CHAR_PER_LINE * LCD_LINES)

/* HD44780 is dead slow and requires delays everywhere */
#if CONF_HD44780_TYPE == HD44780_KS0066U
#define LCD_DELAY_POWER_ON    32000     /* delay in micro seconds after power-on  */
#else
#define LCD_DELAY_POWER_ON    16000     /* delay in micro seconds after power-on  */
#endif
#define LCD_DELAY_INIT        5000      /* delay in micro seconds after initialization sent  */
#define LCD_DELAY_INIT_REP    100       /* delay in micro seconds after initialization repeated */
#define LCD_DELAY_BUSY_FLAG   10        /* delay in micro seconds between busy checks */
#define LCD_DELAY_NO_READBACK 2000      /* delay in micro seconds any operation may take to complete */

/* HD44780 Control and Display commands, taken from the datasheet */
#define LCD_CLEAR             0 /* DB0: Clear display                  */
#define LCD_HOME              1 /* DB1: Cursor home                    */
#define LCD_ENTRY_MODE        2 /* DB2: Entry mode set                 */
#define LCD_ENTRY_INC         1 /* 1=increment, 0=decrement cursor pos */
#define LCD_ENTRY_SHIFT       0 /* 1=display is shifted, 0=not shifted */
#define LCD_ON                3 /* DB3: turn lcd/cursor/blink on/off   */
#define LCD_ON_DISPLAY        2 /* 1=turn display on                   */
#define LCD_ON_CURSOR         1 /* 1=turn cursor on                    */
#define LCD_ON_BLINK          0 /* 1=blinking cursor on                */
#define LCD_SHIFT             4 /* DB4: move cursor/display            */
#define LCD_SHIFT_DISP        3 /* 1=shift display                     */
#define LCD_SHIFT_RIGHT       2 /* 1=right shift                       */
#define LCD_FUNCTION_SET      5 /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4 /* 1=8 bit interface, 0=4 bit          */
#define LCD_FUNCTION_2LINES   3 /* 1=2 line display, 0=1 line display  */
#define LCD_FUNCTION_10DOTS   2 /* 1=5x10 font, 0=5x7 font             */
#define LCD_SET_CGRAM         6 /* DB6: set CG RAM address             */
#define LCD_SET_DDRAM         7 /* DB7: set DD RAM address             */
#define LCD_BUSY              7 /* DB7: 1=busy,0=ready                 */

/* display commands */
#define CMD_CLEAR_DISPLAY() \
  _BV(LCD_CLEAR)
#define CMD_HOME() \
  _BV(LCD_HOME)
#define CMD_ENTRY_MODE(inc, shift) \
  ((uint8_t)(_BV(LCD_ENTRY_MODE) | (inc) << LCD_ENTRY_INC | (shift) << LCD_ENTRY_SHIFT))
#define CMD_DISPLAY(display, cursor, blink) \
  ((uint8_t)(_BV(LCD_ON) | (display) << LCD_ON_DISPLAY | (cursor) << LCD_ON_CURSOR |\
      (blink) << LCD_ON_BLINK))
#define CMD_SHIFT(cursor, right) \
  ((uint8_t)(_BV(LCD_SHIFT) | (cursor) << LCD_SHIFT_DISP | (right) << LCD_SHIFT_RIGHT))
#define CMD_FUNCTIONSET(buswidth, lines, font) \
  ((uint8_t)(_BV(LCD_FUNCTION_SET) | (buswidth) << LCD_FUNCTION_8BIT |\
      (lines) << LCD_FUNCTION_2LINES | (font) << LCD_FUNCTION_10DOTS))
#define CMD_SETCGRAMADR(addr) \
  ((uint8_t)(_BV(LCD_SET_CGRAM) | (addr)))
#define CMD_SETDDRAMADR(addr) \
  ((uint8_t)(_BV(LCD_SET_DDRAM) | (addr)))


/* lcd output stream, used e.g. by the tty module */
FILE lcd = FDEV_SETUP_STREAM(hd44780_put, NULL, _FDEV_SETUP_WRITE);

/* Current backlight state */
#ifdef HD44780_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_POWERON
uint8_t hd44780_backlight_state = 1;
#else
uint8_t hd44780_backlight_state;
#endif
#endif

/* Current cursor position */
static uint8_t current_pos;

/* own prototypes */
static void hd44780_waitbusy(uint8_t en);
static void output_byte(uint8_t rs, uint8_t data, uint8_t en);
#ifdef HD44780_READBACK_SUPPORT
static uint8_t input_byte(uint8_t rs, uint8_t en);
#endif

static void
hd44780_waitbusy(uint8_t en)
{
#ifdef HD44780_READBACK_SUPPORT
  /* check busy flag, with timeout */
  uint8_t busy, timeout = 200;

  busy = input_byte(0, en) & _BV(LCD_BUSY);
  while (busy && timeout > 0)
  {
    _delay_us(LCD_DELAY_BUSY_FLAG);
    busy = input_byte(0, en) & _BV(LCD_BUSY);
    timeout--;
  }

#ifdef DEBUG
  if (timeout == 0)
    debug_printf("lcd timeout! EN=%d\n", en);
#endif

#else /* no HD44780_READBACK_SUPPORT */
  /* no readback, just wait the maximum time a command can take... */
  _delay_us(LCD_DELAY_NO_READBACK);
#endif
}

static void
output_byte(uint8_t rs, uint8_t data, uint8_t en)
{
  /* check busy flag and wait for previous op to complete */
  hd44780_waitbusy(en);

  /* output byte */
  hd44780_output_4bit(rs, HI4(data), en);
  hd44780_output_4bit(rs, LO4(data), en);
}

#ifdef HD44780_READBACK_SUPPORT
static uint8_t
input_byte(uint8_t rs, uint8_t en)
{
  return hd44780_input_4bit(rs, en) << 4 | hd44780_input_4bit(rs, en);
}
#endif

/* Initialize hd44780 driver module */
void
hd44780_init(void)
{
  /* Init connection hardware */
  hd44780_hw_init();
  _delay_us(LCD_DELAY_POWER_ON);

  uint8_t en = 1;

#ifdef HD44780_MULTIEN_SUPPORT
  /* Initialize both controllers */
  do
  {
#endif

#if CONF_HD44780_TYPE == HD44780_KS0066U
    /* Initialize KS0066U for 4 Bit interface
     *
     * NB: this one seems to rely on a proper power-on reset and
     * may or may not work...
     */
    hd44780_output_4bit(0, 0x02, en);
#else /* CONF_HD44780_TYPE <> HD44780_KS0066U */
    /* Initialize HD44780 using instructions for 4 Bit interface,
     * ref. HD44780U datasheet.
     */
    hd44780_output_4bit(0, 0x03, en);
    _delay_us(LCD_DELAY_INIT);

    hd44780_output_4bit(0, 0x03, en);
    _delay_us(LCD_DELAY_INIT_REP);

    hd44780_output_4bit(0, 0x03, en);
    _delay_us(LCD_DELAY_INIT_REP);

    hd44780_output_4bit(0, 0x02, en);   /* 4bit mode */
#endif /*CONF_HD44780_TYPE */

    /* configure for 4 bit, 2 lines, 5x8 font (datasheet, page 24) */
    output_byte(0, CMD_FUNCTIONSET(0, 1, 0), en);

    /* turn on display, cursor and blinking off */
    output_byte(0, CMD_DISPLAY(1, 0, 0), en);

    /* clear display */
    output_byte(0, CMD_CLEAR_DISPLAY(), en);

    /* set shift and increment */
    output_byte(0, CMD_ENTRY_MODE(1, 0), en);

    /* set ddram address */
    output_byte(0, CMD_SETDDRAMADR(0), en);

    DEBUG_LCD(hd44780_init, "init of %S controller done\n",
              (en == 1) ? PSTR("1st") : PSTR("2nd"));

#ifdef HD44780_MULTIEN_SUPPORT
  }
  while (++en < 3);
#endif

  /* set current virtual postion */
  current_pos = 0;
}

/* Configure LCD features cursor and blink */
void
hd44780_config(uint8_t cursor, uint8_t blink)
{
  output_byte(0, CMD_DISPLAY(1, cursor, blink), LCD_CONTR_EN1);

#ifdef HD44780_MULTIEN_SUPPORT
  /* on one display with two independent controllers two visible cursors
   * are pretty useless, but anyway... :-o
   */
  output_byte(0, CMD_DISPLAY(1, cursor, blink), LCD_CONTR_EN2);
#endif
}

/* Define a custom character in CG-RAM */
void
hd44780_define_char(uint8_t n_char, uint8_t * data)
{
  uint8_t en = 1;

  if (n_char > 7)
    return;

#ifdef HD44780_MULTIEN_SUPPORT
  /* define character in both controllers */
  do
  {
#endif

    /* set cgram pointer to char number n */
    output_byte(0, CMD_SETCGRAMADR(n_char * 8), en);
    n_char = 0;
    while (n_char < 8)
    {
      /* send the data to lcd into cgram */
      output_byte(1, *(data + n_char), en);
      n_char++;
    }

#ifdef HD44780_MULTIEN_SUPPORT
  }
  while (++en < 3);
#endif
}

/* Clear display */
void
hd44780_clear(void)
{
  DEBUG_LCD(hd44780_clear, "\n");

  output_byte(0, CMD_CLEAR_DISPLAY(), LCD_CONTR_EN1);

#ifdef HD44780_MULTIEN_SUPPORT
  _delay_us(LCD_DELAY_BUSY_FLAG);
  output_byte(0, CMD_CLEAR_DISPLAY(), LCD_CONTR_EN2);
#endif

  /* set current virtual postion */
  current_pos = 0;
}

/* Cursor home */
void
hd44780_home(void)
{
  DEBUG_LCD(hd44780_home, "\n");

  output_byte(0, CMD_HOME(), LCD_CONTR_EN1);

#ifdef HD44780_MULTIEN_SUPPORT
  output_byte(0, CMD_HOME(), LCD_CONTR_EN2);
#endif

  /* set current virtual postion */
  current_pos = 0;
}

/* Set cursor to line and position (starting with 0) */
void
hd44780_goto(uint8_t line, uint8_t pos)
{
  current_pos = ((line * LCD_CHAR_PER_LINE) + pos);     // % LCD_MAX_CHAR;

  DEBUG_LCD(hd44780_home, "current_pos: %d\n", current_pos);

  /* No need to update DDRAM address.
   * This change has no noticable side effects unless hd44780_put
   * is called and that function will use current_pos is used to
   * calculate the new DDRAM address.
   */
}

/* Shift display one pos to the left or to the right */
void
hd44780_shift(uint8_t right)
{
  output_byte(0, CMD_SHIFT(1, right ? 1 : 0), LCD_CONTR_EN1);

#ifdef HD44780_MULTIEN_SUPPORT
  output_byte(0, CMD_SHIFT(1, right ? 1 : 0), LCD_CONTR_EN2);
#endif
}

/* Put a single character at the current cursor position */
int
hd44780_put(char d, FILE * stream)
{
  uint8_t start = 0;
  uint8_t en = 1;

  if (d == '\n')
  {
    /* move cursor to the next line overwriting previous contents */
    while (current_pos % LCD_CHAR_PER_LINE > 0)
      hd44780_put(' ', stream);

    if (current_pos >= LCD_MAX_CHAR)
      current_pos -= LCD_MAX_CHAR;
    DEBUG_LCD(hd44780_put, "current_pos: %d\n", current_pos);

    return 0;
  }

  if (d == '\r')
  {
    /* carriage return */
    current_pos -= current_pos % LCD_CHAR_PER_LINE;
    DEBUG_LCD(hd44780_put, "current_pos: %d\n", current_pos);

    return 0;
  }

  /* put char at current position */
  if (current_pos <= LCD_CHAR_PER_LINE - 1)
  {
    start = LCD_LINE_1_ADR - 0 + current_pos;
    en = LCD_LINE_1_EN;
  }
  else if (current_pos <= (LCD_CHAR_PER_LINE * 2) - 1)
  {
    start = LCD_LINE_2_ADR - LCD_CHAR_PER_LINE + current_pos;
    en = LCD_LINE_2_EN;
  }
  else if (current_pos <= (LCD_CHAR_PER_LINE * 3) - 1)
  {
    start = LCD_LINE_3_ADR - (LCD_CHAR_PER_LINE * 2) + current_pos;
    en = LCD_LINE_3_EN;
  }
  else if (current_pos <= (LCD_CHAR_PER_LINE * 4) - 1)
  {
    start = LCD_LINE_4_ADR - (LCD_CHAR_PER_LINE * 3) + current_pos;
    en = LCD_LINE_4_EN;
  }

#ifdef HD44780_CHARCONV_SUPPORT
  d = hd44780_charconv(d);
#endif

  output_byte(0, CMD_SETDDRAMADR(start), en);
  output_byte(1, d, en);

  current_pos++;

  if (current_pos == LCD_MAX_CHAR)
    current_pos = 0;
  DEBUG_LCD(hd44780_put, "current_pos: %d\n", current_pos);

  return 0;
}

/*
  -- Ethersex META --
  header(hardware/lcd/hd44780.h)
  init(hd44780_init)
*/
