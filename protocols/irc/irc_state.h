/*
 * Copyright (c) 2009 by Bernd Stellwag <burned@zerties.org>
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

#ifndef HAVE_IRC_STATE_H
#define HAVE_IRC_STATE_H

enum {
    IRC_SEND_INIT,
    IRC_SEND_USERNICK,
    IRC_SEND_ALTNICK,
    IRC_SEND_JOIN,
    IRC_CONNECTED,
};

#include <inttypes.h>
#include "protocols/ecmd/via_tcp/ecmd_state.h"

struct irc_connection_state_t {
    uint8_t stage;
    uint8_t sent;

#ifdef ECMD_IRC_SUPPORT
    uint8_t reparse;
#endif

    char inbuf[ECMD_INPUTBUF_LENGTH];
    char outbuf[ECMD_OUTPUTBUF_LENGTH];
};

#endif  /* HAVE_IRC_STATE_H */
