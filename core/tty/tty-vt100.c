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

#include <stdio.h>

#include "core/tty/tty.h"
#include "core/tty/tty-vt100.h"

char vt100_buf[COLS * LINES];
char *vt100_head = vt100_buf;
const char PROGMEM vt100_clr_str[] = "\033[2J\033[H";

static inline void
vt100_putbuf (char ch)
{
  *(vt100_head ++) = ch;

  if (vt100_head == vt100_end)
    vt100_head = vt100_buf;	/* wrap around. */
}

static inline void
vt100_putstr (char *p)
{
  VT100DEBUG ("vt100_putstr: %s\n", p);
  for (; *p; p ++)
    vt100_putbuf (*p);
}

static inline void
vt100_putstr_P (PGM_P p)
{
  char ch;
  VT100DEBUG ("vt100_putstr_P: %S\n", p);
  for (; (ch = pgm_read_byte (p)); p ++)
    vt100_putbuf (*p);
}


/*
  VT100 low-level implementation
  (these are called from tty.c!)
 */
void
tty_vt100_clear (void)
{
  vt100_putstr_P (vt100_clr_str);
}

void
tty_vt100_goto (uint8_t y, uint8_t x)
{
  char buf[16];

  /* FIXME this CAN be optimized! */
  sprintf_P (buf, PSTR("\033[%i;%iH"), y + 1, x + 1);
  vt100_putstr (buf);
}

void
tty_vt100_put (uint8_t y, uint8_t x, uint8_t ch)
{
  vt100_putbuf (ch);
}
