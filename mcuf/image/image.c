/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include "autoconf.h"
#include "mcuf/mcuf_modul.h"

#ifdef MCUF_IMAGE_SUPPORT
#ifndef GCC
#include <avr/pgmspace.h>
#endif // GCC
#include "image.h"

// 16x16 pixel 1bit image
void mcuf_image(){
	uint8_t x;
	uint8_t y;
	uint8_t i;
	uint8_t data[32];
	memcpy_P(data, imagedata, 32);
	for (y = 0; y < MCUF_MAX_SCREEN_HEIGHT; y++)
		for (x = 0; x < MCUF_MAX_SCREEN_WIDTH; x++) {
			i = (x < 8) ? 1 : 0;
			setPixel(MCUF_MAX_SCREEN_WIDTH - 1 - x, y, 
                                 (data[(y * 2) + i] & (1 << (x % 8) )) ? 3 : 0);
		}
}

#endif //MCUF_IMAGE_SUPPORT
