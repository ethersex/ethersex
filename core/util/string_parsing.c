/*
 * Copyright (c) 2010 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include "core/util/string_parsing.h"

/* Takes a character string and an place for the value, and parses the first integer
 * after all spaces. If there isn't an integer, it will return 0,
 * otherwise the number of parsed characters, including the spaces
 */

uint8_t
next_uint16(char const *cmd, uint16_t * value)
{
  uint8_t found_number = 0;
  char const *old_cmd = cmd;
  *value = 0;

  while (*cmd == ' ')
    cmd++;                      /* Strip the spaces before */

  while (*cmd >= '0' && *cmd <= '9')
  {
    *value = (*value) * 10 + (*cmd) - '0';
    cmd++;
    found_number = 1;
  }

  return found_number ? cmd - old_cmd : 0;
}

/* Takes a character string and an place for the value, and parses the first number
* after all spaces. Number can contain decimal point. The returned integer is
* a fixpoint integer with fixeddigits decimal places. If there isn't an integer, it will return 0,
* otherwise the number of parsed characters, including the spaces
*/
uint8_t
next_int16_fp(char const *cmd, int16_t * value, int8_t fixeddigits)
{
  uint8_t found_number = 0, found_negative = 0, found_decimal = 0;
  char const *old_cmd = cmd;
  *value = 0;
  while (*cmd == ' ')
    cmd++;                      /* Strip the spaces before */
  if (*cmd == '-')
  {
    found_negative = 1;
    cmd++;
  }
  while ((*cmd >= '0' && *cmd <= '9') || (*cmd == '.' && !found_decimal))
  {
    if (*cmd == '.')
      found_decimal = 1;
    else
    {
      if (fixeddigits > 0 || !found_decimal)       /* Stop parsing if fixeddigits are parsed but consume all numbers */
        *value = (*value) * 10 + (*cmd) - '0';
      if (found_decimal)   /* Count numbers after decimal point */
        fixeddigits--;
    }
    cmd++;
    found_number = 1;
  }
/* Fill up with zero */
  while (fixeddigits-- > 0)
    *value = (*value) * 10;
  if (found_number && found_negative)
    *value *= -1;
  return found_number ? cmd - old_cmd : 0;
}


/* Read hexbyte at cmd, strips all spaces before and returns consumed
   characters or 0 on error */
uint8_t
next_hexbyte(char const *cmd, uint8_t * value)
{
  char const *old_cmd = cmd;
  while (*cmd == ' ')
    cmd++;                      /* Strip the spaces before */

  *value = 0;
  uint8_t i;
  for (i = 0; i < 2; i++)
  {
    /* Shift the value for the next round */
    (*value) = (*value) << 4;
    if (*cmd >= '0' && *cmd <= '9')
      *value |= *cmd - '0';
    else if (*cmd >= 'a' && *cmd <= 'f')
      *value |= *cmd - 'a' + 10;
    else if (*cmd >= 'A' && *cmd <= 'F')
      *value |= *cmd - 'A' + 10;
    else
    {
      *value = *cmd;
      return 0;
    }
    cmd++;

  }
  return cmd - old_cmd;
}
