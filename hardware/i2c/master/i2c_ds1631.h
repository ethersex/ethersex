/*
 *
 * Copyright (c) 2010 by Moritz Wenk MoritzWenk@web.de
 *
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

#ifndef _I2C_DS1631_H
#define _I2C_DS1631_H

#define I2C_SLA_DS1631 0x48 // same as for LM75!

uint16_t i2c_ds1631_start_stop(const uint8_t address, const uint8_t startstop); // startstop == 0 -> stop, all other -> start
uint16_t i2c_ds1631_read_temperature(const uint8_t address, int16_t *temp, int16_t *stemp);

#endif /* _I2C_DS1631_H */
