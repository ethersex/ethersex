/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _VNC_STATE_H
#define _VNC_STATE_H

#include "config.h"
#include "vnc_block_factory.h"

typedef enum {
    VNC_STATE_CONNECTED = 0,
    VNC_STATE_SEND_VERSION,
    VNC_STATE_SEND_AUTH,
    VNC_STATE_SEND_CONFIG,
    VNC_STATE_IDLE,
    VNC_STATE_UPDATE,
} vnc_state_t;

#define VNC_UPDATES_SENT_LENGTH (UIP_CONF_BUFFER_SIZE/sizeof(struct gui_block))

struct vnc_connection_state_t {
  uint8_t state;
  uint8_t update_map[VNC_BLOCK_ROWS][VNC_BLOCK_COL_BYTES];
  /* The first dimension of this array is NOT excact */
  uint8_t updates_sent[VNC_UPDATES_SENT_LENGTH][2]; 
};



#endif /* _HTTPD_STATE_H */
