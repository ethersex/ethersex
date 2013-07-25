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

#ifndef HAVE_JABBER_STATE_H
#define HAVE_JABBER_STATE_H

enum
{
  JABBER_INIT,

  JABBER_OPEN_STREAM,
  /* send: <stream:stream ... */
  /* expect: <stream:stream ... */

  JABBER_GET_AUTH,
  /* send: GET <query ...auth */
  /* expect: <password/> */

  JABBER_SET_AUTH,
  /* send: SET <query ...auth */
  /* expect: result */

  JABBER_SET_PRESENCE,
  /* send: presence and request roster */
  /* expect: nothing special */

  JABBER_CONNECTED,
  /* we're set */
};

enum
{
  JABBER_ACTION_NONE,
  JABBER_ACTION_MESSAGE,
  JABBER_ACTION_LAST,
  JABBER_ACTION_VERSION,
  JABBER_ACTION_AVATAR
};

#include <inttypes.h>
#include "protocols/ecmd/via_tcp/ecmd_state.h"

#define TARGET_BUDDY_MAXLEN 40

struct jabber_connection_state_t
{
  uint8_t stage;
  uint8_t sent;
  uint8_t action;
  char actionid[16];

  char target[TARGET_BUDDY_MAXLEN];
  char outbuf[ECMD_OUTPUTBUF_LENGTH];
};

#endif /* HAVE_JABBER_STATE_H */
