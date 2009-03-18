/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef _MCUF_MODUL_H
#define _MCUF_MODUL_H

#ifndef GCC
#include <stdint.h>
#include <util/delay.h>
#include "../config.h"
#include "mcuf.h"
#else
#include "../contrib/mcuf/modsim.h"
#endif // GCC

#ifdef MCUF_MODUL_BORG16_SUPPORT

#ifdef MCUF_MODUL_BORG16_MATRIX_SUPPORT
#define NUM_ROWS MCUF_MAX_SCREEN_WIDTH
#define NUM_COLS MCUF_MAX_SCREEN_HEIGHT
#define STREAMER_NUM 100
#endif

typedef struct {
        unsigned char x;
        unsigned char y;
} pixel;

typedef enum {
	MCUF_MODUL_PLAY_MODE_MANUAL,
	MCUF_MODUL_PLAY_MODE_RANDOM,
	MCUF_MODUL_PLAY_MODE_SEQUENCE
} MCUF_PLAY_MODE;

void setpixel(pixel p, uint8_t color);
#define WAIT(ms) _delay_ms(ms)
#define wait(ms) _delay_ms(ms)
#define random8() rand()

#endif //MCUF_MODUL_BORG16_SUPPORT

struct mcuf_modul_t {
 void (*handler)(void);
};

uint8_t mcuf_play_modul(MCUF_PLAY_MODE play_mode, uint8_t modul);

void mcuf_chess(void);
void mcuf_clean(uint8_t color, uint8_t delay);
void mcuf_spiral(uint8_t delay);

#endif  /* _MCUF_MODUL_H */

