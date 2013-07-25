/*
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

#ifndef _VNC_BLOCK_FACTORY
#define _VNC_BLOCK_FACTORY

#include <inttypes.h>
#include "core/gui/gui.h"

/* We use struct gui_blocks here, please see core/gui/gui.h for details
 */

#define VNC_BLOCK_WIDTH GUI_BLOCK_WIDTH
#define VNC_BLOCK_HEIGHT GUI_BLOCK_HEIGHT

#define VNC_BLOCK_ROWS   32

#define VNC_BLOCK_COLS   32
#define VNC_BLOCK_COL_BYTES (VNC_BLOCK_COLS - 1) / 8 + 1

#define VNC_BLOCK_LENGTH (VNC_BLOCK_WIDTH * VNC_BLOCK_HEIGHT)

#define VNC_SCREEN_WIDTH (VNC_BLOCK_WIDTH * VNC_BLOCK_COLS)
#define VNC_SCREEN_HEIGHT (VNC_BLOCK_HEIGHT * VNC_BLOCK_ROWS)


struct vnc_update_header {
  uint8_t type; /* message type, update == 0 */
  uint8_t padding; /* No Idea what this means */
  uint16_t block_count;
  struct gui_block blocks[];
};


/* x and y are block addresses */
void vnc_make_block(struct gui_block *dest, uint8_t block_x, uint8_t block_y); 

#endif /* _VNC_BLOCK_FACTORY */
