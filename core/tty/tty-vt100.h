/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef TTY_VT100_H
#define TTY_VT100_H

#ifdef DEBUG_TTY_LAYER_MAP
# include "core/debug.h"
# define VT100DEBUG(a...)  debug_printf("vt100: " a)
#else
# define VT100DEBUG(a...)
#endif


#include <avr/pgmspace.h>

extern char vt100_buf[COLS * LINES];
extern char *vt100_head;
#define vt100_end (&vt100_buf[sizeof(vt100_buf)])

/* VT100 escape sequence to clear screen and go home. */
extern const char PROGMEM vt100_clr_str[];

void tty_vt100_clear (void);
void tty_vt100_goto (uint8_t y, uint8_t x);
void tty_vt100_put (uint8_t y, uint8_t x, uint8_t ch);

#endif	/* TTY_VT100_H */
