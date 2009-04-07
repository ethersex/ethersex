/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _MCUF_H
#define _MCUF_H

#include <stdint.h>
#include "config.h"
/* The default baudrate of the shifter is 115200 baud (ubrr == 10 @ 20 Mhz) */
#define MCUF_BAUDRATE 115200
//DEBUG_BAUDRATE

/* MCUF_OUTPUT_SCREEN_... are now defined during Ethersex configuration
   and thus are defined in autoconf.h file.*/
#ifdef MCUF_SERIAL_SUPPORT
#ifndef MCUF_OUTPUT_SUPPORT
   #define MCUF_MIN_SCREEN_WIDTH  MCUF_SERIAL_SCREEN_WIDTH
   #define MCUF_MIN_SCREEN_HEIGHT (0)
   #define MCUF_SPLIT_SCREEN_HEIGHT MCUF_SERIAL_SCREEN_HEIGHT
   #define MCUF_MAX_SCREEN_WIDTH  MCUF_SERIAL_SCREEN_WIDTH
   #define MCUF_MAX_SCREEN_HEIGHT MCUF_SERIAL_SCREEN_HEIGHT
#else /* not MCUF_OUTPUT_SUPPORT */
 #if MCUF_SERIAL_SCREEN_WIDTH > MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MIN_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
 #else
   #define MCUF_MIN_SCREEN_WIDTH  MCUF_SERIAL_SCREEN_WIDTH
 #endif
 #if MCUF_SERIAL_SCREEN_HEIGHT > (MCUF_OUTPUT_SCREEN_HEIGHT/2)
   #define MCUF_MIN_SCREEN_HEIGHT  (MCUF_OUTPUT_SCREEN_HEIGHT/2)
   #define MCUF_SPLIT_SCREEN_HEIGHT (MCUF_OUTPUT_SCREEN_HEIGHT/2)
 #else
   #define MCUF_MIN_SCREEN_HEIGHT  MCUF_SERIAL_SCREEN_HEIGHT
   #define MCUF_SPLIT_SCREEN_HEIGHT MCUF_SERIAL_SCREEN_HEIGHT
 #endif
 #if MCUF_SERIAL_SCREEN_WIDTH > MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MAX_SCREEN_WIDTH  MCUF_SERIAL_SCREEN_WIDTH
 #else
   #define MCUF_MAX_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
 #endif
 #if MCUF_SERIAL_SCREEN_HEIGHT > MCUF_OUTPUT_SCREEN_HEIGHT
   #define MCUF_MAX_SCREEN_HEIGHT  MCUF_SERIAL_SCREEN_HEIGHT
 #else
   #define MCUF_MAX_SCREEN_HEIGHT  MCUF_OUTPUT_SCREEN_HEIGHT
 #endif   
#endif /* not MCUF_OUTPUT_SUPPORT */
#else /* MCUF_SERIAL_SUPPORT */
#ifdef BLP_SUPPORT
   #define MCUF_MIN_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MIN_SCREEN_HEIGHT (0)
   #define MCUF_SPLIT_SCREEN_HEIGHT MCUF_OUTPUT_SCREEN_HEIGHT
   #define MCUF_MAX_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MAX_SCREEN_HEIGHT MCUF_OUTPUT_SCREEN_HEIGHT
#endif
#ifdef LEDRG_SUPPORT
   #define MCUF_MIN_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MIN_SCREEN_HEIGHT (MCUF_OUTPUT_SCREEN_HEIGHT/2)
   #define MCUF_SPLIT_SCREEN_HEIGHT (MCUF_OUTPUT_SCREEN_HEIGHT/2)
   #define MCUF_MAX_SCREEN_WIDTH  MCUF_OUTPUT_SCREEN_WIDTH
   #define MCUF_MAX_SCREEN_HEIGHT MCUF_OUTPUT_SCREEN_HEIGHT
#endif
#endif /* MCUF_SERIAL_SUPPORT */

#ifdef MCUF_SUPPORT
void mcuf_init(void);
void mcuf_newdata(void);
void mcuf_periodic(void);
void mcuf_show_clock(uint8_t clockswitch);
void mcuf_show_string(char *);

void draw_box(uint8_t startx, uint8_t starty, uint8_t lengthx, uint8_t lengthy,
               uint8_t outercolor, uint8_t innercolor);
void setPixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t getPixel(uint8_t x, uint8_t y);

extern uint8_t gdata[MCUF_MAX_SCREEN_HEIGHT][MCUF_MAX_SCREEN_WIDTH];

struct mcuf_scrolltext_struct {
  uint8_t tomove;
  uint8_t posshift;
  uint8_t end;
  uint8_t posx;
  uint8_t posy;
  uint8_t color;
  uint8_t bcolor;
  uint8_t waittime;
};

extern struct mcuf_scrolltext_struct mcuf_scrolltext_buffer;
#endif  /* MCUF_SUPPORT */


#endif  /* _MCUF_H */
