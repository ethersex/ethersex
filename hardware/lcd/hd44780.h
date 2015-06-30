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

#ifndef _HD44780_H
#define _HD44780_H

#include <stdio.h>
#include "config.h"

#ifdef HD44780_SUPPORT

#define HD44780_ORIGINAL 1
#define HD44780_DISPTECH 2
#define HD44780_KS0067B  3
#define HD44780_KS0066U  4
#define HD44780_WDC2704  5
#define HD44780_SPLC780D 6

#define HD44780_DIREKT 16
#define HD44780_SERLCD 17
#define HD44780_I2CSUPPORT 18


/*Definition for different display types
  Add some new Displays here*/
#if CONF_HD44780_TYPE == HD44780_ORIGINAL
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x20
    #define LCD_LINE_3_ADR 0x40
    #define LCD_LINE_4_ADR 0x60
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 1
    #define LCD_LINE_4_EN 1
#elif CONF_HD44780_TYPE == HD44780_DISPTECH
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x10
    #define LCD_LINE_4_ADR 0x50
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 1
    #define LCD_LINE_4_EN 1
#elif CONF_HD44780_TYPE == HD44780_KS0067B
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x14
    #define LCD_LINE_4_ADR 0x54
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 1
    #define LCD_LINE_4_EN 1
#elif CONF_HD44780_TYPE == HD44780_KS0066U
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x10
    #define LCD_LINE_4_ADR 0x50
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 1
    #define LCD_LINE_4_EN 1
#elif CONF_HD44780_TYPE == HD44780_WDC2704
    #define LCD_CHAR_PER_LINE 27 	
    #define LCD_LINES 4 		
    #define LCD_LINE_1_ADR 0x00
    #define LCD_LINE_2_ADR 0x40
    #define LCD_LINE_3_ADR 0x00
    #define LCD_LINE_4_ADR 0x40
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 2
    #define LCD_LINE_4_EN 2
#elif CONF_HD44780_TYPE == HD44780_SPLC780D
    #define LCD_CHAR_PER_LINE 16
    #define LCD_LINES 4
    #define LCD_LINE_1_ADR 0x80
    #define LCD_LINE_2_ADR 0xC0
    #define LCD_LINE_3_ADR 0x90
    #define LCD_LINE_4_ADR 0xD0
    #define LCD_LINE_1_EN 1
    #define LCD_LINE_2_EN 1
    #define LCD_LINE_3_EN 1
    #define LCD_LINE_4_EN 1
#else
#error "unknown hd44780 compatible controller type!"

#endif

/* lcd stream */
extern FILE lcd;
extern uint8_t current_pos;
#ifdef HD44780_BACKLIGHT_SUPPORT
extern uint8_t back_light;
#endif

#define clock_write(en) clock_rw(0,en)
#define clock_read(en) clock_rw(1,en)

#define noinline __attribute__((noinline))

/* prototypes */
void hd44780_init(void);
void hd44780_config(uint8_t cursor, uint8_t blink, uint8_t en);
void hd44780_define_char(uint8_t n_char, uint8_t *data, uint8_t en);
#ifdef HD44780_BACKLIGHT_SUPPORT
void hd44780_backlight(uint8_t state);
#endif
void noinline hd44780_clear();
void noinline hd44780_home(void);
void noinline hd44780_goto(uint8_t line, uint8_t pos);
void noinline hd44780_shift(uint8_t right, uint8_t en);
int noinline hd44780_put(char d, FILE *stream);
void noinline hd44780_hw_init(void);
uint8_t noinline clock_rw(uint8_t read, uint8_t en);
void noinline output_nibble(uint8_t rs, uint8_t nibble, uint8_t en );
#ifdef HD44780_READBACK
uint8_t noinline input_nibble(uint8_t rs, uint8_t en);
#endif

#endif /* HD44780_SUPPORT */

#endif /* _HD44780_H */
