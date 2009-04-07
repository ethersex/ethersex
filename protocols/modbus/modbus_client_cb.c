/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#define MODBUS_HOLD_REGISTERS 11
#define MODBUS_INPUT_REGISTERS 7

#ifdef MODBUS_CLIENT_SUPPORT

uint16_t holding[MODBUS_HOLD_REGISTERS] = {0x2342, 0xf0f0,0xdead,0xbeef};
uint16_t
modbus_read_holding(uint8_t address) 
{
  return holding[address];
}

void
modbus_write_holding(uint8_t address, uint16_t value) 
{
  holding[address] = value;
}

uint16_t
modbus_read_input(uint8_t address) 
{
  return 0xf0f0;
}
#endif
