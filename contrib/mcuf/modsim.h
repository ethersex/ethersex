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

#ifndef GCC
  #error "no GCC compiler used"
#endif

#ifndef _MODSIM_H
#define _MODSIM_H

#define GCC 1
#define MCUF_MAX_SCREEN_WIDTH 16
#define MCUF_MAX_SCREEN_HEIGHT 16
#define PINB 0 // something should be done with this pin

#define debug_printf printf
#define uint8_t int
#define uint16_t int
#define memcpy_P memcpy
#define PROGMEM
#define NULL 0 

void setPixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t getPixel(uint8_t x, uint8_t y);
void _delay_ms(uint8_t delay);

#endif /* _MODSIM_H */

