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

/*
 * Parts of this file are derived from Open HR20 LCD Driver, which is
 * distributed unter GPLv2+ and
 *
 * Copyright (C) 2008 Dario Carluccio (hr20-at-carluccio-dot-de)
 *		 2009 Thomas Vosshagen (mod. for THERMOTronic) (openhr20-at-vosshagen-dot-com)
 */

#include <avr/pgmspace.h>

// Look-up table to convert value to LCD display data (segment control)
uint8_t hr20_charset[] PROGMEM =
{
    0x3F, //      0:0x3F   1:0x06   2:0x5B   3:0x4F   4:0x66   5:0x6D   6:0x7D
    0x06, // 1    ******        *   ******   ******   *    *   ******   ******
    0x5B, // 2    *    *        *        *        *   *    *   *        *
    0x4F, // 3    *    *        *   ******   ******   ******   ******   ******
    0x66, // 4    *    *        *   *             *        *        *   *    *
    0x6D, // 5    ******        *   ******   ******        *   ******   ******
    0x7D, // 6   0111111  0000110  1011011  1001111  1100110  1101101  1111101

    0x07, // 7    7:0x07   8:0x7F   9:0x6F  10:0x77  11:0x7C  12:0x39  13:0x5E
    0x7F, // 8    ******   ******   ******   ******   *        ******        *
    0x6F, // 9         *   *    *   *    *   *    *   *        *             *
    0x77, //10         *   ******   ******   ******   ******   *        ******
    0x7C, //11         *   *    *        *   *    *   *    *   *        *    *
    0x39, //12         *   ******   ******   *    *   ******   ******   ******
    0x5E, //13   0000111  1111111  1101111  1110111  1111100  0111001  1011110

    0x79, //14   14:0x79  15:0x71  16:0x63  17:0x54  18:0x73  19:0x76 20:0x30
    0x71, //15    ******   ******   ******            ******   *    *  *
    0x63, //16    *        *        *    *            *    *   *    *  *
    0x54, //17    ******   ******   ******   ******   ******   ******  *
    0x73, //18    *        *                 *    *   *        *    *  *
    0x76, //19    ******   *                 *    *   *        *    *  *
    0x30, //20   1111001  1110001  1100011  1010100  1110011  1110110  0110000

    0x08, //21   21:0x08  22:0x40  23:0x01  24:0x48  25:0x41  26:0x09  27:0x49
    0x40, //22                      ******            ******   ******   ******
    0x01, //23
    0x48, //24             ******            ******   ******            ******
    0x41, //25
    0x09, //26    ******                     ******            ******   ******
    0x49, //27   0001000  1000000  0000001  1001000  1000001  0001001  1001001

    0x50, //28   28:0x50           30:0x10  31:0x5c  32:0x00  33:0x33  34:0x27
          //                                                   ******   ******
    0x10, //30                                                 *    *   *    *
    0x5c, //31    ******            *        ******   Space    *    *   *    *
    0x00, //32    *                 *        *    *            *             *
    0x33, //33    *                 *        ******            *             *
    0x27, //34   1010000           0010000  1011100  0000000  0110011  0100111

    0x38, //35   35:0x38  36:0x58  37:0x3e  38:0x78  39:0x6e
    0x58, //36    *                 *    *   *        *    *
    0x3e, //37    *                 *    *   *        *    *
    0x78, //38    *        ******   *    *   ******   ******
    0x6e  //39    *        *        *    *   *             *
          //40    ******   ******   ******   ******   ******
          //41   0111000  1011000  0111110  1111000  1101110
};


// Look-up table to adress element F for one Position.
uint8_t hr20_nf_offsets[] PROGMEM =
{
	LCD_SEG_0F,
	LCD_SEG_1F,
	LCD_SEG_2F,
	LCD_SEG_3F,
};

// Look-up table to adress a segment inside a field (relative to xF)
uint8_t hr20_seg_offsets[] PROGMEM =
{
	LCD_SEG_0A - LCD_SEG_0F,	//  Seg A            AAAA
	LCD_SEG_0B - LCD_SEG_0F,	//  Seg B           F    B
	LCD_SEG_0C - LCD_SEG_0F,	//  Seg C           F    B
	LCD_SEG_0D - LCD_SEG_0F,	//  Seg D            GGGG
	LCD_SEG_0E - LCD_SEG_0F,	//  Seg E           E    C
	0,				//  Seg F           E    C
	LCD_SEG_0G - LCD_SEG_0F,	//  Seg G            DDDD
};

//! Look-up table for adress hour-bar segments
uint8_t hr20_bar_offsets[] PROGMEM =
{
    LCD_SEG_B0,    LCD_SEG_B1,    LCD_SEG_B2,    LCD_SEG_B3,    LCD_SEG_B4,
    LCD_SEG_B5,    LCD_SEG_B6,    LCD_SEG_B7,    LCD_SEG_B8,    LCD_SEG_B9,
    LCD_SEG_B10,   LCD_SEG_B11,   LCD_SEG_B12,   LCD_SEG_B13,   LCD_SEG_B14,
    LCD_SEG_B15,   LCD_SEG_B16,   LCD_SEG_B17,   LCD_SEG_B18,   LCD_SEG_B19,
    LCD_SEG_B20,   LCD_SEG_B21,   LCD_SEG_B22,   LCD_SEG_B23
};

