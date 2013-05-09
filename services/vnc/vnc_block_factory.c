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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <math.h>
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "vnc.h"
#include "core/gui/gui.h"
#include "vnc_state.h"
#include "vnc_block_factory.h"

#include "config.h"


void
vnc_make_block(struct gui_block *dest, uint8_t block_x, uint8_t block_y)
{
    /* This is only for the helper functions */
    dest->x = block_x;
    dest->y = block_y;
    memset(dest->data, 0xff, sizeof(dest->data));

#ifdef GUI_SUPPORT
    /* This calls the matek layer to draw the picture into the memory
       area */
    matek_draw(dest);
#else
    uint8_t x, y;
    for (x = 0; x < VNC_BLOCK_WIDTH; x++)
        for (y = 0; y < VNC_BLOCK_HEIGHT; y++) 
            dest->data[x * VNC_BLOCK_WIDTH + y] = x + y;
#endif


    dest->x = HTONS(block_x * VNC_BLOCK_WIDTH);
    dest->y = HTONS(block_y * VNC_BLOCK_HEIGHT);
    dest->w = HTONS(VNC_BLOCK_WIDTH);
    dest->h = HTONS(VNC_BLOCK_HEIGHT);
    dest->encoding = 0;
}
