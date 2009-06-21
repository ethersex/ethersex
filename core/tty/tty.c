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
#include <stdlib.h>

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
  uint8_t i = win->maxx - win->x + 1;

  while (i --)
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
  TTYDEBUG ("wclrtobot: from y=%d, x=%d, maxy=%d, maxx=%d\n",
	    _y, _x, win->maxy, win->maxx);

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
  if (! win->subwin)
    {
      /* Yippie, root window, clear it all up ... */
      TTYDEBUG ("clearing root-window.\n");
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
  TTYDEBUG ("wscroll: lines=%d, maxy=%d\n", lines, win->maxy);

  if (lines > win->maxy)
    {
      wclear (win);		/* Cursor home. */
      return;
    }

  for (uint8_t y = 0; y <= win->maxy - lines; y ++)
    {
      TTYDEBUG ("wscroll: copying y=%d\n", y);
      wmove (win, y, 0);
      for (uint8_t x = 0; x <= win->maxx; x ++)
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
  TTYDEBUG ("waddch: ch='%c', y=%d, x=%d\n", ch, win->y, win->x);

  if (win->y > win->maxy)
    {
      TTYDEBUG ("waddch: y=%d, x=%d, need to scroll, ok=%d\n",
		win->y, win->x, win->scrollok);

      /* Cursor out of window, ... */
      if (! win->scrollok)
	return;			/* ... cannot help. */

      wscroll (win, win->y - win->maxy);
    }

  /* Update off-screen map. */
  switch (ch)
    {
    case '\n':			/* Newline */
      win->y ++;

      /* Fall through. */
    case '\r':			/* Return */
      win->x = 0;
      tty_ll_goto (win->y + win->begy, win->x + win->begx);
      break;

    default:			/* Print everything else. */
      TTYDEBUG ("  -> map[%2d,%2d] = '%c'\n", win->y + win->begy,
		win->x + win->begx, ch);
      map (win, win->y, win->x) = ch;
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
      break;
    }

}

void
waddstr (WINDOW *win, const char *ptr)
{
  for (; *ptr; ptr ++)
    waddch (win, *ptr);
}

void
waddstr_P (WINDOW *win, PGM_P ptr)
{
  uint8_t ch;
  for (; (ch = pgm_read_byte (ptr)); ptr ++)
    waddch (win, ch);
}

void
wmove (WINDOW *win, uint8_t y, uint8_t x)
{
  if (y > win->maxy || x > win->maxx)
    return;			/* Out of range. */

  TTYDEBUG ("wmove'ing to y=%d, x=%d\n", y, x);
  win->y = y;
  win->x = x;

  tty_ll_goto (y + win->begy, x + win->begx);
}

void
wprintw (WINDOW *win, const char *fmt, ...)
{
  char buf[COLS + 1];

  va_list va;
  va_start (va, fmt);

  TTYDEBUG ("wprintw: win=%p, %s to y=%d,x=%d ", win, fmt, win->y, win->x);
  vsnprintf (buf, COLS + 1, fmt, va);

  TTYDEBUG ("--> '%s'\n", buf);
  waddstr (win, buf);

  va_end (va);
}

WINDOW *
subwin (WINDOW *win, uint8_t lines, uint8_t cols, uint8_t begy, uint8_t begx)
{
  WINDOW *newwin = malloc (sizeof (WINDOW));
  if (! newwin) return NULL;

  newwin->subwin = 1;
  newwin->linewrap = 1;
  newwin->scrollok = 1;

  newwin->y = 0;
  newwin->x = 0;

  newwin->maxy = lines - 1;
  newwin->maxx = cols - 1;
  newwin->begy = begy;
  newwin->begx = begx;

  return newwin;
}


/*
  -- Ethersex META --
  header(core/tty/tty.h)
  initearly(initscr)
*/
