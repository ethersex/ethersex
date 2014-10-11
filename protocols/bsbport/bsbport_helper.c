/*
*
* Copyright (c) 2013 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include <util/crc16.h>

#include "config.h"
#include "bsbport.h"
#include "bsbport_helper.h"


// Generates CCITT XMODEM CRC from message
uint16_t
bsbport_crc(uint8_t * buffer, uint8_t length)
{
  uint16_t crc = 0, i;

  for (i = 0; i < length; i++)
  {
    crc = _crc_xmodem_update(crc, buffer[i]);
  }

  // Complete message returns 0x00
  // Message w/o last 2 bytes (CRC) returns last 2 bytes (CRC)
  return crc;
}

// Convert value inside message (pointer to 2 bytes) Integer
int16_t
bsbport_ConvertToInt16(uint8_t * msg)
{
  // Temperatures are SIGNED INT 16-Bit in width
  int16_t converted = 0;
  converted = ((int16_t) msg[0] << 8) + (int16_t) msg[1];
  return converted;
}

// Convert temperature inside message to FixPoint Integer
float
bsbport_ConvertToTemp(int16_t raw)
{
  // Temperatures are SIGNED INT 16-Bit in width
  return (float) (raw) / 64;
}

// Convert value to FixPoint Integer
float
bsbport_ConvertToFP1(int16_t raw)
{
  return (float) (raw) / 10;
}

// Convert value to FixPoint Integer
float
bsbport_ConvertToFP5(int16_t raw)
{
  return (float) (raw) / 2;
}

// Convert temperature and store it inside message (pointer to 2 bytes)
void
bsbport_ConvertTempToData(float Temp, uint8_t * Target)
{
  int16_t Val = Temp * 64;

  Target[0] = (Val >> 8);
  Target[1] = Val & 0xFF;
}
