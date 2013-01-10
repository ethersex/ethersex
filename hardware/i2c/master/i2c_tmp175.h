/*
 *
 * Copyright (c) 2011 by Patrick Hieber <patrick.hieber@gmx.net>
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

#ifndef _I2C_TMP175_H
#define _I2C_TMP175_H

// The default chip address (if all pins are GND)
#define I2C_SLA_TMP175 0x48
// The data register, wherein the temperature bits reside.
#define I2C_SLA_TMP175_DR 0x00

int16_t i2c_tmp175_read_temp(uint8_t address);

#endif /* _I2C_TMP175_H */
