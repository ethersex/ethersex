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
#include "vnc_state.h"
#include "vnc_block_factory.h"
#include "vnc_font.h"

#include "config.h"

void 
vnc_putchar(struct vnc_block *dest,
            char data, 
            uint8_t color, 
            uint8_t char_line, 
            uint8_t char_column) 
{
    uint8_t x, y;
    /* We have to select the right line */
    if (char_line / 2 != dest->y) return;

/*
  ______
  ______
  _XX___
  ___X__
  _XXX__
  X__X__
  _XXX__
  ______ 
   ^  ^
   |  + char_x_offset + char_x_len;
   +- char_x_offset
 */
    /* Now we select the right row */
    uint16_t tmp = char_column * 6;
    if (! (tmp / VNC_BLOCK_WIDTH == dest->x)  
        && ! ((tmp / VNC_BLOCK_WIDTH == (dest->x - 1) && tmp > (dest->x * VNC_BLOCK_WIDTH - 6)))) return;

    uint8_t char_x_offset, char_x_len;

    /* Offset within the block */
    uint8_t x_offset;

    /* Start Pixel of character in block */
    if (tmp >= dest->x * VNC_BLOCK_WIDTH) {
        char_x_offset = 0;
        x_offset = tmp % VNC_BLOCK_WIDTH;
        if (tmp + 6 > (dest->x + 1) * VNC_BLOCK_WIDTH)
            char_x_len =  (dest->x + 1) * VNC_BLOCK_WIDTH - tmp;
        else
            char_x_len = 6;
    } else {
        /* Start Pixel is in the block before */
        char_x_offset = dest->x * VNC_BLOCK_WIDTH - tmp;
        char_x_len = 6 - char_x_offset;
        x_offset = 0;
    } 
        

    for (x = 0; x <  char_x_len; x++) {
        for (y = 0; y < 8; y++) {
            if (vnc_font[(uint8_t)data][char_x_offset + x] & _BV(y)) {
                dest->data[((char_line % 2) * 8 + y) * VNC_BLOCK_WIDTH + x + x_offset ] = color;
            }
        }
    }
}

void
vnc_make_block(struct vnc_block *dest, uint8_t block_x, uint8_t block_y)
{
    /* This is only for the helper functions */
    dest->x = block_x;
    dest->y = block_y;
	
    uint8_t x, y;
    memset(dest->data, 0xff, sizeof(dest->data));


    char data[] = "HALLO WELT!!!";
    uint8_t len = strlen("HALLO WELT!!!");
    for (x = 0; x < len; x++)
        vnc_putchar(dest, data[x], 0, 1, x);
    for (x = 0; x < len; x++)
        vnc_putchar(dest, data[x], 0, 0, x);

    dest->x = HTONS(block_x * VNC_BLOCK_WIDTH);
    dest->y = HTONS(block_y * VNC_BLOCK_HEIGHT);
    dest->w = HTONS(VNC_BLOCK_WIDTH);
    dest->h = HTONS(VNC_BLOCK_HEIGHT);
    dest->encoding = 0;

    
}
