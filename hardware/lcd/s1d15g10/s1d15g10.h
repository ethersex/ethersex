/*
 *
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

/*
 * Nokia 6100 LCD driver
 *   based on Sparkfun code.
 *   http://www.sparkfun.com/datasheets/LCD/SFE_lcd_driver_c.zip
 *
 * Slightly modified
 * - use only standard data types (uint8_t and uint16_t) no more char and int.
 * - modified initialization.
 * - added handling for RST
 *
 * Pinout for SPI is:
 * PORTD
 *   7: CS
 *   6: SCLK
 *   5: SDATA
 * PORTB
 *   0: RST
 *
 * 	Remember if running this code on a 5V AVR, make sure to add resistors in 
 * 	series so as not to blow the LCD!
 */
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#ifndef LCD_S1D15G10_H
#define LCD_S1D15G10_H

#define lctCmd 0
#define lctData 1

#include "bunnies.h"

void init_lcd(void);
void lcd_setup_window(uint8_t sx, uint8_t sy, uint8_t ex, uint8_t ey);
void fillrect(uint8_t sx, uint8_t sy, uint8_t dx, uint8_t dy, uint8_t color);
void setup_pix_blit(uint8_t sx, uint8_t sy, uint8_t dx, uint8_t dy);
void pix_blit(uint8_t color);
void sendByte(uint8_t cmd, uint8_t data);
uint16_t cpxSetAndValidateLCDRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy);

uint8_t putch(uint8_t x, uint8_t y, char c, uint8_t fg, uint8_t bg);
uint8_t putstr_pgm(uint8_t x, uint8_t y, PGM_P p, uint8_t fg, uint8_t bg);
uint8_t putstr_ram(uint8_t x, uint8_t y, char * p, uint8_t fg, uint8_t bg);
uint8_t printDecimal(uint8_t x, uint8_t y, uint32_t val, uint8_t fg, uint8_t bg);

void lcd_putch(char d);

void lcd_set_brightness_parms(uint8_t a, uint8_t b);

#endif /* LCD_S1D15G10_H */
