/*
* Fixedpoint utils
*
* Copyright (c) 2009 by Gerd v. Egidy <gerd@egidy.de>
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

#include <avr/io.h>

#include "config.h"
#include "core/debug.h"
#include "core/util/fixedpoint.h"

// Attention: returns the length in bytes, not a pointer like the regular itoa
// this is more conveniant for use in output to ECMDs output buffer 
uint8_t itoa_fixedpoint(int16_t n, uint8_t fixeddigits, char s[])
{
    uint8_t i=0, j=0, sign=0, size=0;

    if (n < 0)
    {
        /* record sign */
        sign=1;
        /* make n positive */
        n = -n;
    }

    do
    {
        /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
        if (i == fixeddigits && fixeddigits != 0)
            s[i++]='.';
    }
    while ((n /= 10) > 0);     /* delete it */

    if (i < fixeddigits)
    {
        while(i < fixeddigits)
            s[i++]='0';
        s[i++]='.';
    }

    if (sign)
        s[i++] = '-';
    s[i] = '\0';

    size=i;

    // in-place reverse
    i--;
    while(j<i)
    {
        sign = s[j];
        s[j] = s[i];
        s[i] = sign;
        i--;
        j++;
    }

    return size;
}

