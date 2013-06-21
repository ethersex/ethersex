/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef HR20_LCD_H
#define HR20_LCD_H

#include <inttypes.h>

void hr20_lcd_init (void);
void hr20_lcd_putchar (uint8_t pos, uint8_t ch);
void hr20_lcd_hourbar (uint8_t start, uint8_t stop);

#define LCD_SEG_SET(i)		((&LCDDR0)[(i)/8] |=  (1 << ((i) & 7)))
#define LCD_SEG_CLEAR(i)	((&LCDDR0)[(i)/8] &= ~(1 << ((i) & 7)))
#define LCD_SEG_TOGGLE(i)	((&LCDDR0)[(i)/8] ^=  (1 << ((i) & 7)))

extern const uint8_t hr20_charset[];
extern const uint8_t hr20_nf_offsets[];
extern const uint8_t hr20_seg_offsets[];
extern const uint8_t hr20_bar_offsets[];

enum {
  HR20_LCD_CHAR_0,
  HR20_LCD_CHAR_1,
  HR20_LCD_CHAR_2,
  HR20_LCD_CHAR_3,
  HR20_LCD_CHAR_4,
  HR20_LCD_CHAR_5,
  HR20_LCD_CHAR_6,

  HR20_LCD_CHAR_7,
  HR20_LCD_CHAR_8,
  HR20_LCD_CHAR_9,
  HR20_LCD_CHAR_A,
  HR20_LCD_CHAR_B,
  HR20_LCD_CHAR_C,
  HR20_LCD_CHAR_D,

  HR20_LCD_CHAR_E,
  HR20_LCD_CHAR_F,
  HR20_LCD_CHAR_DEG,
  HR20_LCD_CHAR_N,
  HR20_LCD_CHAR_P,
  HR20_LCD_CHAR_H,
  HR20_LCD_CHAR_I,

  HR20_LCD_CHAR_R,
  HR20_LCD_CHAR_L,
  HR20_LCD_CHAR_U,
  HR20_LCD_CHAR_Y,
  HR20_LCD_CHAR_O,
  HR20_LCD_CHAR_c,
  HR20_LCD_CHAR_S,

  HR20_LCD_CHAR_LDASH,
  HR20_LCD_CHAR_MDASH,
  HR20_LCD_CHAR_HDASH,
  HR20_LCD_CHAR_LMDASH,
  HR20_LCD_CHAR_MHDASH,
  HR20_LCD_CHAR_LHDASH,
  HR20_LCD_CHAR_LHMDASH,

  HR20_LCD_CHAR_J,
  HR20_LCD_CHAR_SPACE
};

#endif  /* HR20_LCD_H */

