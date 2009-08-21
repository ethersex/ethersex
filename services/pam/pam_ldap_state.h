/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef HAVE_LDAP_AUTH_STATE_H
#define HAVE_LDAP_AUTH_STATE_H

#include <inttypes.h>

struct ldap_auth_connection_state_t {
    uint8_t pending; /* We have to sent data */

    /* The packet ID we expect next. */
    uint16_t msgid;
    char username[16];
    char password[16];
    uint8_t *auth_state;
};

#endif  /* HAVE_LDAP_AUTH_STATE_H */
