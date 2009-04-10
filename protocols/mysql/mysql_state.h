/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef HAVE_MYSQL_STATE_H
#define HAVE_MYSQL_STATE_H

enum {
    MYSQL_WAIT_GREETING,
    /* send: nothing */
    /* expect: Server Greeting */

    MYSQL_SEND_LOGIN,
    /* send: login request */
    /* expect: OK */

    MYSQL_CONNECTED,
    /* we're set */
};

#include <inttypes.h>

#define MYSQL_STMTBUF_LEN	60

struct mysql_connection_state_t {
    uint8_t stage;
    uint8_t sent;

    /* The packet ID we expect next. */
    uint16_t packetid;

    union {
	char stmtbuf[MYSQL_STMTBUF_LEN];
	unsigned char seed[20];
    } u;
};

#endif  /* HAVE_MYSQL_STATE_H */
