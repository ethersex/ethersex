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
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "vnc.h"
#include "vnc_state.h"
#include "vnc_block_factory.h"

#include "config.h"

void
vnc_make_block(struct vnc_block *dest, uint8_t x, uint8_t y)
{
    dest->x = HTONS(x * VNC_BLOCK_WIDTH);
    dest->y = HTONS(y * VNC_BLOCK_HEIGHT);
    dest->w = HTONS(VNC_BLOCK_WIDTH);
    dest->h = HTONS(VNC_BLOCK_HEIGHT);
    dest->encoding = 0;
    
    memset(dest->data, rand() % 2 ? rand() : 0, VNC_BLOCK_LENGTH);
}
