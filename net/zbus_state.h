/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef ZBUS_STATE_H
#define ZBUS_STATE_H

#define ZBUS_BUFFER_LEN 25

enum zbus_connection_state {
  ZBUS_STATE_DATA = 1,
  ZBUS_STATE_SENDING = 2,
  ZBUS_STATE_RECIEVED = 4,
};

struct zbus_connection_state_t {
  uint8_t state; /* state of the connection */
  uint8_t offset; /* used by the zbus hardware subsystem */
  uint8_t ttl; /* The connection will be closed after n polls */

  uint8_t buffer_len;
  uint8_t buffer[ZBUS_BUFFER_LEN];
};

#endif
