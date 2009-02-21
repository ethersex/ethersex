/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef _MCUF_H
#define _MCUF_H

/* The default baudrate of the shifter is 115200 baud (ubrr == 10 @ 20 Mhz) */
#define MCUF_BAUDRATE 115200
//DEBUG_BAUDRATE

void mcuf_init(void);
void mcuf_newdata(void);
void mcuf_periodic(void);
void mcuf_show_clock(void);
void mcuf_show_string(char *);

extern uint8_t gdata[MCUF_OUTPUT_SCREEN_HEIGHT][MCUF_OUTPUT_SCREEN_WIDTH];

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
#endif /* _MODBUS_H */
