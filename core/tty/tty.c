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
#include <string.h>

#include "core/tty/tty.h"

uint8_t tty_image[LINES * COLS];
WINDOW tty_mainwin;

#define map(win,y,x)	(tty_image[((y) + (win)->begy) * COLS +	\
				   ((x) + (win)->begx)])

void
initscr (void)
{
  TTYDEBUG ("initializing tty layer ...\n");
  curscr->linewrap = 1;
  curscr->scrollok = 1;
  curscr->maxx = COLS - 1;
  curscr->maxy = LINES - 1;

  tty_ll_clear ();
}

/* Override until end of line, don't care for the cursor. */
static void
wspacetoeol (WINDOW *win)
{
  for (; win->x <= win->maxx; )
    waddch (win, ' ');
}

void
wclrtoeol (WINDOW *win)
{
  uint8_t _y = win->y, _x = win->x;
  wspacetoeol (win);
  wmove (win, _y, _x);		/* Restore cursor. */
}

void
wclrtobot (WINDOW *win)
{
  uint8_t _y = win->y, _x = win->x;

  /* Clear current row. */
  wspacetoeol (win);

  /* Clear lines below. */
  for (uint8_t y = _y + 1; y <= win->maxy; y ++)
    {
      wmove (win, y, 0);
      wspacetoeol (win);
    }

  wmove (win, _y, _x);		/* Restore cursor. */
}

void
wclear (WINDOW *win)
{
  if (! win->parent)
    {
      /* Yippie, root window, clear it all up ... */
      memset (tty_image, 32, LINES * COLS);
      tty_ll_clear ();
      win->y = 0;
      win->x = 0;

      return;
    }

  wmove (win, 0, 0);
  wclrtobot (win);
}

void
wscroll (WINDOW *win, uint8_t lines)
{
  if (lines > win->maxy)
    {
      wclear (win);		/* Cursor home. */
      return;
    }

  for (uint8_t y = 0; y <= win->maxy - lines; y ++)
    {
      wmove (win, y, 0);
      for (; win->x <= win->maxx;)
	waddch (win, map (win, y + lines, win->x)); /* Copy content. */
    }

  /* Finally position the cursor to the beginning of the first clear
     line and clear till the end. */
  wmove (win, win->maxy - lines + 1, 0);
  wclrtobot (win);		/* Restores cursor. */
}

void
waddch (WINDOW *win, const char ch)
{
  TTYDEBUG ("waddch: win=%p, ch=%c\n", win, ch);

  if (win->y > win->maxy)
    {
      /* Cursor out of window, ... */
      if (! win->scrollok)
	return;			/* ... cannot help. */

      wscroll (win, win->y - win->maxy);
    }

  /* Update off-screen map. */
  map (win, win->y + win->begy, win->x + win->begx) = ch;
  tty_ll_put (win->y + win->begy, win->x + win->begx, ch);

  if (win->x == win->maxx)
    {
      /* Cursor reached end of line. */
      if (win->linewrap)
	{
	  win->y ++;
	  win->x = 0;
	}

      tty_ll_goto (win->y + win->begy, win->begx);
    }
  else
    win->x ++;			/* Now need to tty_ll_goto. */
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
  if (y > win->maxy || x > win->maxx)
    return;			/* Out of range. */

  win->y = y;
  win->x = x;

  tty_ll_goto (y + win->begy, x + win->begx);
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
