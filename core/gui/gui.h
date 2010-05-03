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

#ifndef _GUI_GUI_H
#define _GUI_GUI_H

#include <stdint.h>
#define GUI_BLOCK_WIDTH 16
#define GUI_BLOCK_HEIGHT 16
#define GUI_BLOCK_LENGTH (GUI_BLOCK_WIDTH * GUI_BLOCK_HEIGHT)

/* This is the same as vnc_block for padding reasons */
struct gui_block {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint32_t encoding;
  uint8_t data[GUI_BLOCK_LENGTH];
};

#define GUI_CIRCLE_QUADRANT1 1
#define GUI_CIRCLE_QUADRANT2 2
#define GUI_CIRCLE_QUADRANT3 4
#define GUI_CIRCLE_QUADRANT4 8

#define GUI_CIRCLE_FULL      0x0f

#define GUI_CIRCLE_FILL      16


void gui_draw_circle(struct gui_block *dest, uint16_t cx, uint16_t cy, uint8_t r,
                     uint8_t color, uint8_t quadrant_mask);

/* Interface to the current selected scene */
void matek_draw(struct gui_block *dest);
#endif
