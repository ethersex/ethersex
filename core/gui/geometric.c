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

#include "gui.h"

void
gui_draw_circle(struct gui_block *dest, uint16_t cx, uint16_t cy, uint8_t r, 
                uint8_t color, uint8_t quadrant_mask) {

    
    if (dest->x >= (cx - r)/GUI_BLOCK_WIDTH 
        && dest->x <= (cx + r)/GUI_BLOCK_WIDTH 
	    && dest->y >= (cy - r)/GUI_BLOCK_WIDTH 
        && dest->y <= (cy + r)/GUI_BLOCK_WIDTH) {

		uint8_t _x, _y;
        quadrant_mask = ~quadrant_mask;

		for (_x = 0; _x < GUI_BLOCK_WIDTH; _x++) {
			for (_y = 0; _y < GUI_BLOCK_HEIGHT; _y++) {
				uint16_t x = dest->x * GUI_BLOCK_WIDTH + _x;
				uint16_t y = dest->y * GUI_BLOCK_HEIGHT + _y;
				int16_t dx = x - cx;
				int16_t dy = cy - y;


                if ((quadrant_mask & GUI_CIRCLE_QUADRANT1) && dx >= 0 && dy >= 0) 
                    continue;
                if ((quadrant_mask & GUI_CIRCLE_QUADRANT2) && dx < 0 && dy > 0) 
                    continue;
                if ((quadrant_mask & GUI_CIRCLE_QUADRANT3) && dx <= 0 && dy <= 0)
                    continue;
                if ((quadrant_mask & GUI_CIRCLE_QUADRANT4) && dx > 0 && dy < 0)
                    continue;
				uint32_t hh = dx * dx + dy * dy;
				uint32_t rr = r * r;

				if (hh < rr)
                    if (!(quadrant_mask & GUI_CIRCLE_FILL) 
                        || (hh > ((r - 2) * (r - 2))))
                        dest->data[_y * GUI_BLOCK_WIDTH + _x] = color;
			}
		}
	}
}
