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

#ifndef MCUF_IMAGE_H
#define MCUF_IMAGE_H

#ifdef MCUF_IMAGE_SUPPORT

static uint8_t imagedata[] PROGMEM = {
0x00,0x00,0x00,0x00,
0x0f,0xf0,0x0f,0xf0,
0x0c,0x00,0x0c,0x00,
0x0c,0x00,0x0c,0x00,
0x0f,0xf0,0x0f,0xf0,
0x0c,0x00,0x0c,0x00,
0x0c,0x00,0x0f,0xf0,
0x0f,0xf0,0x00,0x00
};

void mcuf_image(void);

#endif //MCUF_IMAGE_SUPPORT

#endif /* MCUF_IMAGE_H */
