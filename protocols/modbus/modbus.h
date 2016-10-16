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

#ifndef _MODBUS_H
#define _MODBUS_H

struct modbus_buffer {
  uint8_t *data;
  uint8_t sent;
  uint8_t len;
  uint16_t crc;
  uint8_t crc_len;
};

void modbus_init(void);
void modbus_periodic(void);
uint8_t modbus_rxstart(uint8_t *data, uint8_t len, int16_t *recv_len);
uint16_t modbus_crc_calc(uint8_t *data, uint8_t len);

#endif /* _MODBUS_H */
