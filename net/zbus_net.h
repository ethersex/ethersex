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

#ifndef ZBUS_NET_H
#define ZBUS_NET_H

/* constants */
#define ZBUS_PORT 23514

enum zbus_udp_answers {
  ZBUS_UDP_OK,
  ZBUS_UDP_ERROR,
  ZBUS_UDP_ERROR_TOO_MUCH_DATA,
  ZBUS_UDP_ERROR_TOO_MUCH_CONNECTIONS,
  ZBUS_UDP_ERROR_OLD_DATA,
};

/* prototypes */
void zbus_net_init(void);
void zbus_net_main(void);

#endif /* ZBUS_NET_H */
