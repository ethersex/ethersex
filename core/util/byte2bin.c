/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright(c) by Michael Brakemeier <michael@brakemeier.de>
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

#include "byte2bin.h"

char *
byte2bin(uint8_t value)
{
  static char binstr[9];
  uint8_t i;

  binstr[8] = '\0';
  for (i = 0; i < 8; i++)
  {
    binstr[7 - i] = value & 1 ? '1' : '0';
    value = value / 2;
  }
  return binstr;
}
