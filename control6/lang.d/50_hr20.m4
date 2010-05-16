dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

divert(globals_divert)dnl
#include "hardware/lcd/hr20.h"
#include "hardware/adc/hr20-temp.h"

divert(-1)dnl

dnl ==========================================================================
dnl HR20_SHOW_TEMP(TEMP)
dnl ==========================================================================
define(`HR20_SHOW_TEMP', `{
	int16_t hr20_st_val = $1;
	if(hr20_st_val<100)
		hr20_lcd_putchar(3, HR20_LCD_CHAR_SPACE);
	else
		hr20_lcd_putchar(3, hr20_st_val / 100);

	hr20_lcd_putchar(2, (hr20_st_val / 10) % 10);
	hr20_lcd_putchar(1, (hr20_st_val % 10));
	hr20_lcd_putchar(0, HR20_LCD_CHAR_DEG);

	LCD_SEG_SET (LCD_SEG_COL1);
	LCD_SEG_CLEAR (LCD_SEG_COL2);
}')


dnl ==========================================================================
dnl HR20_SHOW_VOLTAGE(VOLTAGE)
dnl ==========================================================================
define(`HR20_SHOW_VOLTAGE', `{
	int16_t hr20_st_val = $1;
	hr20_lcd_putchar(3, HR20_LCD_CHAR_SPACE);
	hr20_lcd_putchar(2, (hr20_st_val / 100));
	hr20_lcd_putchar(1, (hr20_st_val / 10) % 10);
	hr20_lcd_putchar(0, (hr20_st_val % 10));

	LCD_SEG_SET (LCD_SEG_COL1);
	LCD_SEG_CLEAR (LCD_SEG_COL2);
}')
