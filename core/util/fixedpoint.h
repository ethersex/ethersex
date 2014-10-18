/*
* fixedpoint utils
*
* Copyright (c) 2009 Gerd v. Egidy <gerd@egidy.de>
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef _UTIL_FIXEDPOINT_H_
#define _UTIL_FIXEDPOINT_H_

#include <stdint.h>

/* Attention: returns the length in bytes, not a pointer like the regular
 * itoa this is more convenient for use in output to ECMDs output buffer.
 * Function needs an integer, the number of fixeddigits, a pointer to
 * outputbuffer and size of outputbuffer. Its Callers responsibility to
 * allocate memory for outputbuffer */
uint8_t itoa_fixedpoint(int16_t n, uint8_t fixeddigits, char s[],
                        uint8_t size);

#endif
