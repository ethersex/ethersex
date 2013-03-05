/*
* byte to HEX utils
*
* Copyright (c) 2013 by Frank Sautter <ethersix@sautter.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 3 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 675 Mass
* Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include <avr/io.h>

#include "config.h"
#include "core/util/byte2hex.h"

/* this could easily be done by using sprintf, but the lib has a much to big
 * memory footprint, so we convert the data into a hex string on our own */

uint8_t byte2hex (uint8_t value, char *string)
{
  // convert high nibble into hex ascii
  string[0] = (value >> 4) + '0';
  if(string[0] > '9')
    string[0] += 'A' - '0' - 10;

  // convert low nibble into hex ascii
  string[1] = (value & 0X0F) + '0';
  if (string[1] > '9')
    string[1] += 'A' - '0' - 10;

  return (2);
}
