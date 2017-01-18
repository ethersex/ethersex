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

#ifndef _HD44780_H
#define _HD44780_H

#include <stdint.h>
#include <stdio.h>

#include "config.h"

#ifdef HD44780_SUPPORT

/*
 * LCD size/type
 */
#define HD44780_TYPE_8C_1L  1
#define HD44780_TYPE_16C_1L 2
#define HD44780_TYPE_16C_1L_MUX 3
#define HD44780_TYPE_20C_1L 4
#define HD44780_TYPE_40C_1L 5
#define HD44780_TYPE_8C_2L  6
#define HD44780_TYPE_12C_2L 7
#define HD44780_TYPE_16C_2L 8
#define HD44780_TYPE_20C_2L 9
#define HD44780_TYPE_24C_2L 10
#define HD44780_TYPE_40C_2L 11
#define HD44780_TYPE_16C_4L 12
#define HD44780_TYPE_20C_4L 13
#define HD44780_TYPE_20C_4L_KS0073 14
#define HD44780_TYPE_27C_4L 15
#define HD44780_TYPE_40C_4L 16

/*
 * LCD controller type
 *
 * Most alphanumeric displays use HD44780 compatible controllers.
 * If your display controller really requires special commands then
 * add it here and in config.in and add the necessary code to
 * hd44780_core.h.
 *
 * DO NOT add displays with compatible controllers but different sizes
 * or address mappings here. Add them to the display type definitions
 * below instead!
 */
#define HD44780_CONTR_COMPAT  1
#define HD44780_CONTR_KS0066U 2

/*
 * Connection type
 */
#define HD44780_DIREKT 1
#define HD44780_I2CSUPPORT 2
#define HD44780_SERLCD 3
#define HD44780_2WIRE 4

/*
 * I2C Port Expander
 */
#define HD44780_I2C_PCF8574 1
#define HD44780_I2C_MCP23017 2

/*
 * Constants for multi-enable EN
 */
#define LCD_CONTR_EN1 1
#define LCD_CONTR_EN2 2

/*
 * Definition for different display types.
 *
 * To add further displays just add another definition
 * with correct number of lines, characters per line
 * and address mapping. Everything else is computed.
 * Do not forget to add a corresponding define above and
 * to the choice in config.in.
 *
 * LCDs can have the same size, but use different start addresses.
 * PLEASE, READ THE DATASHEET!
 */
#if CONF_HD44780_TYPE == HD44780_TYPE_8C_1L
#define LCD_CHAR_PER_LINE 8
#define LCD_LINES 1
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x00
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_16C_1L
#define LCD_CHAR_PER_LINE 18
#define LCD_LINES 1
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x00
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_16C_1L_MUX
/* 1 line, but non-contiguous adresses!
 * Needs special handling as a 2 line device...
 * Note: atm no display for testing available - this
 * one will problably not work as expected.
 */
#define LCD_CHAR_PER_LINE 8
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_20C_1L
#define LCD_CHAR_PER_LINE 20
#define LCD_LINES 1
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x00
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_40C_1L
#define LCD_CHAR_PER_LINE 40
#define LCD_LINES 1
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x00
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_8C_2L
#define LCD_CHAR_PER_LINE 8
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_12C_2L
#define LCD_CHAR_PER_LINE 12
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_16C_2L
/* e.g. Data Vision 16244 S1FBLY */
#define LCD_CHAR_PER_LINE 16
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_20C_2L
#define LCD_CHAR_PER_LINE 20
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_24C_2L
#define LCD_CHAR_PER_LINE 24
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_40C_2L
#define LCD_CHAR_PER_LINE 40
#define LCD_LINES 2
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x00
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_16C_4L
/* e.g. Tinsharp TC1604 (Pollin)*/
#define LCD_CHAR_PER_LINE 16
#define LCD_LINES 4
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x10
#define LCD_LINE_4_ADR 0x50
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_20C_4L
/* e.g. TC2004A (Pollin) */
#define LCD_CHAR_PER_LINE 20
#define LCD_LINES 4
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x14
#define LCD_LINE_4_ADR 0x54
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_20C_4L_KS0073
/* address mapping differs from default */
#define LCD_CHAR_PER_LINE 20
#define LCD_LINES 4
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x20
#define LCD_LINE_3_ADR 0x40
#define LCD_LINE_4_ADR 0x60
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN1
#define LCD_LINE_4_EN LCD_CONTR_EN1
#elif CONF_HD44780_TYPE == HD44780_TYPE_27C_4L
/* e.g. WDC2704
 * Display has two controllers, requires MULTIEN_SUPPORT
 */
#define LCD_CHAR_PER_LINE 27
#define LCD_LINES 4
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x40
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN2
#define LCD_LINE_4_EN LCD_CONTR_EN2
#elif CONF_HD44780_TYPE == HD44780_TYPE_40C_4L
/* e.g. Displaytech 404B
 * Display has two controllers, requires MULTIEN_SUPPORT
 */
#define LCD_CHAR_PER_LINE 40
#define LCD_LINES 4
#define LCD_LINE_1_ADR 0x00
#define LCD_LINE_2_ADR 0x40
#define LCD_LINE_3_ADR 0x00
#define LCD_LINE_4_ADR 0x40
#define LCD_LINE_1_EN LCD_CONTR_EN1
#define LCD_LINE_2_EN LCD_CONTR_EN1
#define LCD_LINE_3_EN LCD_CONTR_EN2
#define LCD_LINE_4_EN LCD_CONTR_EN2
#else
#error "unknown hd44780 display type!"
#endif

/* lcd output stream, used e.g. by the tty module */
extern FILE lcd;

/* Current backlight state */
#ifdef HD44780_BACKLIGHT_SUPPORT
extern uint8_t hd44780_backlight_state;
#endif

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

/*
 * API for HD44780-based alphanumeric LCDs
 */

/* Initialize hd44780 driver module */
extern void hd44780_init(void);

/* Configure LCD features cursor and blink */
extern void hd44780_config(uint8_t cursor, uint8_t blink);

/* Define a custom character in CG-RAM */
extern void hd44780_define_char(uint8_t n_char, uint8_t * data);

/* Clear display */
extern void noinline hd44780_clear(void);

/* Cursor home without changing the display contents */
extern void noinline hd44780_home(void);

/* Set cursor to line and position (starting with 0) */
extern void noinline hd44780_goto(uint8_t line, uint8_t pos);

/* Shift display one pos to the left or to the right */
extern void noinline hd44780_shift(uint8_t right);

/* Put a single character d at the current cursor position */
extern int noinline hd44780_putc(char c, FILE * stream);

/* Write the string pointed to by str at the current cursor position */
extern int noinline hd44780_puts(const char *str, FILE * stream);

/* Variant of hd44780_puts() where str resides in program memory */
extern int noinline hd44780_puts_P(const char *str, FILE * stream);

#ifdef HD44780_CHARCONV_SUPPORT
extern char hd44780_charconv(char from);
#endif

/* Connection low-level interface.
 *
 * These functions provide a hardware abstraction from the LCD interface
 * and have to be implemented per connection type (direct, i2c, ...).
 */

/* Initialize connection circuitry */
extern void noinline hd44780_hw_init(void);

/* Transfer data to the LCD */
extern uint8_t noinline hd44780_clock_rw(uint8_t read, uint8_t en);

/* Write data to LCD */
extern void noinline hd44780_output_4bit(uint8_t rs, uint8_t data,
                                         uint8_t en);
#ifdef HD44780_READBACK_SUPPORT
/* Read data from LCD */
extern uint8_t noinline hd44780_input_4bit(uint8_t rs, uint8_t en);
#endif

#ifdef HD44780_BACKLIGHT_SUPPORT
/* Switch backlight on/off */
extern void hd44780_backlight(uint8_t state);
#endif

/* provide some short-cuts */
#define hd44780_clock_write(en) hd44780_clock_rw(0,en)
#define hd44780_clock_read(en) hd44780_clock_rw(1,en)

#endif /* HD44780_SUPPORT */

#endif /* _HD44780_H */
