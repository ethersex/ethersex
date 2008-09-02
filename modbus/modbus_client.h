/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef _MODBUS_CLIENT_H
#define _MODBUS_CLIENT_H


void modbus_client_process(uint8_t *data, uint8_t length, struct modbus_connection_state_t *state);

union ModbusRTU {
    uint8_t raw[MODBUS_BUFFER_LEN];
    /* FC04/FC03 */
    struct {
      uint8_t addr;
      uint8_t cmd;
      uint16_t ptr;
      uint16_t len;
      uint16_t crc;
    } read;
    /* FC06 */
    struct {
      uint8_t addr;
      uint8_t cmd;
      uint16_t ptr;
      uint16_t value;
      uint16_t crc;
    } write;
    /* FC16 */
    struct {
      uint8_t addr;
      uint8_t cmd;
      uint16_t ptr;
      uint16_t len;
      uint8_t byte_count;
      uint16_t data[];
    } xwrite;
};


#endif /* _MODBUS_H */
