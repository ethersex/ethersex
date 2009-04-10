/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef _MCUF_MODUL_H
#define _MCUF_MODUL_H

#include "autoconf.h"
#ifndef GCC
#include <stdint.h>
#include <util/delay.h>
#include "config.h"
#include "mcuf.h"
#else
#include "contrib/mcuf/modsim.h"
#endif // GCC

#ifdef MCUF_MODUL_BORG16_SUPPORT

#define NUM_ROWS MCUF_MAX_SCREEN_WIDTH
#define NUM_COLS MCUF_MAX_SCREEN_HEIGHT
#define LINEBYTES (((NUM_COLS-1)/8)+1)

#define STREAMER_NUM 100
#define FEUER_N 5
#define FEUER_S 30
#define FEUER_DIV 44
#define FEUER_DELAY 50
#define FEUER_Y (NUM_ROWS + 3)

#define SNAKE_DELAY 100

// disabled as long there is no joystick support
#define JOYISFIRE  0 //(!(PIND & (1<<BITFIRE)))
#define JOYISLEFT  0 //(!(PINB & (1<<BITLEFT)))
#define JOYISRIGHT 0 //(!(PINB & (1<<BITRIGHT)))
#define JOYISDOWN  0 //(!(PINB & (1<<BITDOWN)))
#define JOYISUP    0 //(!(PINB & (1<<BITUP)))


typedef struct {
        uint8_t x;
        uint8_t y;
} pixel;

typedef enum {right,left,up,down} direction;
typedef struct {
        pixel pos;
        direction dir;
        enum{clear=0, set=1} mode;
} cursor;

void clear_screen(uint8_t color);
unsigned char get_pixel(pixel p);
unsigned char get_next_pixel(pixel p, direction dir);
direction direction_r(direction dir);
pixel next_pixel(pixel pix, direction dir);
void set_cursor(cursor* cur, pixel p);
void walk(cursor* cur, unsigned char steps, unsigned int delay);

void clear_screen(uint8_t color);
unsigned char get_pixel(pixel p);
unsigned char get_next_pixel(pixel p, direction dir);
direction direction_r(direction dir);
pixel next_pixel(pixel pix, direction dir);
void set_cursor(cursor* cur, pixel p);
void walk(cursor* cur, unsigned char steps, unsigned int delay);

void setpixel(pixel p, uint8_t color);
#define WAIT(ms) _delay_ms(ms)
#define wait(ms) _delay_ms(ms)
#define random8() rand()
#define clearpixel(p) setpixel(p, 0);

#endif //MCUF_MODUL_BORG16_SUPPORT

typedef enum {
	MCUF_MODUL_PLAY_MODE_MANUAL,
	MCUF_MODUL_PLAY_MODE_RANDOM,
	MCUF_MODUL_PLAY_MODE_SEQUENCE
} MCUF_PLAY_MODE;

uint8_t mcuf_list_modul(char* title, uint8_t modul);
uint8_t mcuf_play_modul(MCUF_PLAY_MODE play_mode, uint8_t modul);

#endif  /* _MCUF_MODUL_H */

