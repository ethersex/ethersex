/*
 * hd44780 driver library
 *
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

#include <stdint.h>

#include <avr/pgmspace.h>

#include "config.h"

#include "hd44780.h"

#if CONF_HD44780_CHARSET == HD44780_CHARSET_COMPAT
/*
 * "Traditional" HD44780 charset conversion table
 *
 * Conversions:
 * - map missing backslash to slash '\' to '/'
 * - tilde is missing, no conv possible, gives '->'
 * - map umlaut accent characters to the corresponding umlaut characters
 * - map other accent characters to the characters without accents
 * - map beta (=sharp s), micro and Yen
 * - map 'n/N with tilde' to 'n with bar above'
 *
 */
static const unsigned char hd44780_charmap[] PROGMEM = {
  /* #0 */
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 126, 127, 28, 29, 30, 31,
  /* #32 */
  32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55,
  56, 57, 58, 59, 60, 61, 62, 63,
  /* #64 */
  64, 65, 66, 67, 68, 69, 70, 71,
  72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87,
  88, 89, 90, 91, 47, 93, 94, 95,
  /* #96 */
  96, 97, 98, 99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  /* #128 */
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  /* #160 */
  160, 33, 236, 237, 164, 92, 124, 167,
  34, 169, 170, 171, 172, 173, 174, 175,
  223, 177, 178, 179, 39, 228, 247, 165,
  44, 185, 186, 187, 188, 189, 190, 63,
  /* #192 */
  65, 65, 65, 65, 225, 65, 65, 67,
  69, 69, 69, 69, 73, 73, 73, 73,
  68, 238, 79, 79, 79, 79, 239, 120,
  48, 85, 85, 85, 245, 89, 240, 226,
  /* #224 */
  97, 97, 97, 97, 225, 97, 97, 99,
  101, 101, 101, 101, 105, 105, 105, 105,
  111, 238, 111, 111, 111, 111, 239, 253,
  48, 117, 117, 117, 245, 121, 240, 255
};
#endif

char
hd44780_charconv(char from)
{
  return pgm_read_byte(&(hd44780_charmap[(uint8_t) from]));
}
