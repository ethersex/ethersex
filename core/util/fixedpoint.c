/*
* Fixedpoint utils
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

#include <stdint.h>

#include "config.h"
#include "fixedpoint.h"

/* Attention: returns the length in bytes, not a pointer like the regular
 * itoa this is more convenient for use in output to ECMDs output buffer */
uint8_t
itoa_fixedpoint(int16_t n, uint8_t fixeddigits, char s[], uint8_t size)
{
  uint8_t len = 0;
  if (size <= 1)
    return 0;

  if (n < 0)
  {
    s[len++] = '-';
    n = -n;
  }

/* Number of digits to output */
/* Output at least fixeddigits + 1 digits */
  uint8_t digits = 1;
  int16_t m = 1;
  while ((m <= n / 10) || (digits < fixeddigits + 1))
  {
    m *= 10;
    digits++;
  }

  size--;
  while (digits > 0 && len < size)
  {
    uint8_t i;
/* Decimal point? */
    if (digits == fixeddigits)
    {
      s[len++] = '.';
    }
    for (i = '0'; n >= m; n -= m, i++);
    if (len < size)
      s[len++] = i;
    m /= 10;
    digits--;
  }

  s[len] = '\0';

  return len;
}
