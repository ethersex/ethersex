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

#include "mcuf_modul.h"

#ifdef MCUF_CHESS_SUPPORT
void mcuf_chess()
{
  uint8_t y;
  uint8_t x;
  for (y=0; y < MCUF_MAX_SCREEN_WIDTH; y++)
    for (x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++)
       setPixel(x, y, x + y);
}
#endif //MCUF_CHESS_SUPPORT

#ifdef MCUF_CLEAN_SUPPORT
void mcuf_clean(uint8_t color, uint8_t delay)
{
  uint8_t y;
  uint8_t x;
  for (y=0; y < MCUF_MAX_SCREEN_WIDTH; y++){
    for (x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++) {
       setPixel(x, y, 0);
      _delay_ms(delay);
     }
  }
}
#endif //MCUF_CLEAN_SUPPORT

#ifdef MCUF_SPIRAL_SUPPORT
void mcuf_spiral(uint8_t delay)
{
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t x1 = 0;
  uint8_t y1 = 0;
  uint8_t x2 = MCUF_MAX_SCREEN_WIDTH-1;
  uint8_t y2 = MCUF_MAX_SCREEN_HEIGHT-1;

  while ( (x1 <= x2) && (y1 <= y2)){
    for (x=x1; x < x2; x++) {
      setPixel(x, y, 1);
      _delay_ms(delay);
    }
    x1++;
    for (y=y1; y < y2; y++){
      setPixel(x, y, 2);
      _delay_ms(delay);
    }
    y1++;
    for (x=x2; x > x1; x--) {
      setPixel(x, y, 3);
      _delay_ms(delay);
    }
    x2--;
    for (y=y2; y > y1; y--){
      setPixel(x, y, 1);
      _delay_ms(delay);
    }
    y2--;
  } 
}

#endif //MCUF_SPIRAL_SUPPORT

