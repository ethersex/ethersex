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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "core/tty/tty.h"

uint8_t tty_image[LINES * COLS];
WINDOW tty_mainwin;
uint8_t tty_ll_y, tty_ll_x;

/* The input fifo queue.  Fill it with _getch_queue. */
static char tty_input_queue[8];

#define map(win,y,x)	(tty_image[((y) + (win)->begy) * COLS +	\
				   ((x) + (win)->begx)])


inline static void
tty_ll_clear (void)
{
#ifdef TTY_LL_HD44780
  tty_hd44780_clear ();
#endif
#ifdef TTY_LL_S1D15G10
  tty_s1d15g10_clear ();
#endif
#ifdef TTY_LL_S1D13305
  tty_s1d13305_clear ();
#endif
#ifdef TTY_LL_VT100_TELNET
  tty_vt100_clear ();
#endif
}

inline static void
tty_ll_goto (uint8_t y, uint8_t x)
{
  if (y == tty_ll_y && x == tty_ll_x)
    return;			/* No change. */

#ifdef TTY_LL_HD44780
  tty_hd44780_goto (y, x);
#endif
#ifdef TTY_LL_VT100_TELNET
  tty_vt100_goto (y, x);
#endif
#ifdef TTY_LL_S1D13305
  tty_s1d13305_goto(x, y);
#endif

  tty_ll_y = y;
  tty_ll_x = x;
}

inline static void
tty_ll_put (uint8_t y, uint8_t x, uint8_t ch)
{
  TTYDEBUG_MAP ("tty_ll_put: %c to %i,%i, cursor is at %i,%i \n",
		ch, y, x, tty_ll_y, tty_ll_x);

  if (y != tty_ll_y || x != tty_ll_x)
    tty_ll_goto (y, x);

#ifdef TTY_LL_HD44780
  tty_hd44780_put (y, x, ch);
#endif
#ifdef TTY_LL_S1D15G10
  tty_s1d15g10_put (y, x, ch);
#endif
#ifdef TTY_LL_S1D13305
  tty_s1d13305_put (x, y, &ch);
#endif
#ifdef TTY_LL_VT100_TELNET
  tty_vt100_put (y, x, ch);
#endif

  tty_ll_x ++;
}


void
initscr (void)
{
  TTYDEBUG ("initializing tty layer ...\n");
  curscr->linewrap = 1;
  curscr->scrollok = 1;
  curscr->leaveok = 1;
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
      TTYDEBUG_MAP ("wscroll: copying y=%d\n", y);
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
  TTYDEBUG_MAP ("waddch: w=%p, ch='%c', y=%d, x=%d\n",
		win, ch, win->y, win->x);

  if (win->y > win->maxy)
    {
      TTYDEBUG_MAP ("waddch: y=%d, x=%d, need to scroll, ok=%d\n",
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
      if (!win->leaveok)
	tty_ll_goto (win->y + win->begy, win->x + win->begx);
      break;

    default:			/* Print everything else. */
      TTYDEBUG_MAP ("  -> map[%2d,%2d] = '%c'\n", win->y + win->begy,
		    win->x + win->begx, ch);
      if (map (win, win->y, win->x) != ch)
	{
	  map (win, win->y, win->x) = ch;
	  tty_ll_put (win->y + win->begy, win->x + win->begx, ch);
	}

      if (win->x == win->maxx)
	{
	  /* Cursor reached end of line. */
	  if (win->linewrap)
	    {
	      win->y ++;
	      win->x = 0;
	    }

	  if (!win->leaveok)
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

  if (!win->leaveok)
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


void
wprintw_P (WINDOW *win, const char *fmt, ...)
{
  char buf[COLS + 1];

  va_list va;
  va_start (va, fmt);

  TTYDEBUG ("wprintw: win=%p, %s to y=%d,x=%d ", win, fmt, win->y, win->x);
  vsnprintf_P (buf, COLS + 1, fmt, va);

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
  newwin->leaveok = 1;

  newwin->y = 0;
  newwin->x = 0;

  newwin->maxy = lines - 1;
  newwin->maxx = cols - 1;
  newwin->begy = begy;
  newwin->begx = begx;

  TTYDEBUG ("new subwin (%p): pos %i:%i, size %ix%i\n",
	    newwin, begy, begx, cols, lines);
  return newwin;
}

char
getch (void)
{
  char ch = tty_input_queue[0];

  if (ch)
    {
      uint8_t len = strlen (tty_input_queue);
      memmove (tty_input_queue, tty_input_queue + 1, len);

      TTYDEBUG ("getch: de-queueing char '%c' (= %d)\n", ch, ch);
    }

  return ch;
}

uint8_t
_getch_queue (char ch)
{
  uint8_t len = strlen (tty_input_queue);

  if (len + 1 < sizeof (tty_input_queue))
    {
      TTYDEBUG ("getch: pushing char '%c' (= %d) to queue.\n", ch, ch);

      tty_input_queue[len] = ch;
      tty_input_queue[len + 1] = 0;

      return 0;
    }

  return 1;			/* buffer space exceeded */
}


/*
  -- Ethersex META --
  header(core/tty/tty.h)
  init(initscr)
*/
