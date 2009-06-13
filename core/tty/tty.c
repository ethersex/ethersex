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
#include <hardware/lcd/hd44780.h>

uint8_t tty_image[LINES][COLS];
WINDOW tty_mainwin;

void
initscr (void)
{
  TTYDEBUG ("initializing tty layer ...\n");
  curscr->linewrap = 1;
  curscr->maxx = COLS - 1;
  curscr->maxy = LINES - 1;

  hd44780_clear ();
}

void
waddch (WINDOW *win, const char ch)
{
  TTYDEBUG ("waddch: win=%p, ch=%c\n", win, ch);
  putc (ch, lcd);
}

void
waddstr (WINDOW *win, const char *ptr)
{
  for (; *ptr; ptr ++)
    waddch (win, *ptr);
}

void
wmove (WINDOW *win, uint8_t y, uint8_t x)
{
  hd44780_goto (y, x);
}

void
wprintw (WINDOW *win, const char *fmt, ...)
{
  int redir_helper (char d, FILE *stream)
  {
    waddch (NULL, d);
    return 0;
  }

  FILE redir = FDEV_SETUP_STREAM(redir_helper, NULL, _FDEV_SETUP_WRITE);

  va_list va;
  va_start (va, fmt);

  TTYDEBUG ("wprintw: win=%p, %s\n", win, fmt);
  vfprintf (&redir, fmt, va);
  va_end (va);
}



/*
  -- Ethersex META --
  header(core/tty/tty.h)
  initearly(initscr)
*/
