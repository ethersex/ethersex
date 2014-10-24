/*
 * Copyright (c) 2010 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <stdint.h>

/* Takes a character string and an place for the value, and parses the first integer
 * after all spaces. If there isn't an integer, it will return 0,
 * otherwise the number of parsed characters, including the spaces
 */

uint8_t next_uint16(char const *cmd, uint16_t * value);
#define next_int16(cmd, value) next_int16_fp( cmd, value, 0 )

/* Takes a character string and an place for the value, and parses the first number
* after all spaces. Number can contain decimal point. The returned integer is
* a fixpoint integer with fixeddigits decimal places. If there isn't an integer, it will return 0,
* otherwise the number of parsed characters, including the spaces
*/

uint8_t next_int16_fp(char const *cmd, int16_t * value, int8_t fixeddigits);

/* Read hexbyte at cmd, strips all spaces before and returns consumed
   characters or 0 on error */
uint8_t next_hexbyte(char const *cmd, uint8_t * value);

