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

#ifndef TTY_H
#define TTY_H

#include <inttypes.h>
#include "config.h"
#define LINES	TTY_LINES
#define COLS	TTY_COLS

#ifdef DEBUG_TTY_LAYER
# include "core/debug.h"
# define TTYDEBUG(a...)  debug_printf("tty: " a)
#else
# define TTYDEBUG(a...)
#endif

#ifdef DEBUG_TTY_LAYER_MAP
# include "core/debug.h"
# define TTYDEBUG_MAP(a...)  debug_printf("tty: " a)
#else
# define TTYDEBUG_MAP(a...)
#endif

/* The (one and only) off-screen image. */
extern uint8_t tty_image[LINES * COLS];

/* Low-level cursor position. */
extern uint8_t tty_ll_y, tty_ll_x;

/* WINDOW type forward declaration. */
struct _tty_window_t;
typedef struct _tty_window_t WINDOW;

struct _tty_window_t {
  /* Whether this window is a sub-window. */
  unsigned subwin		:1;

  /* Whether to wrap the cursor at the end of the line or not. */
  unsigned linewrap		:1;

  /* Whether scrolling of the window is allowed. */
  unsigned scrollok		:1;

  /* Whether the cursor can be left whereever or not. */
  unsigned leaveok		:1;

  /* Current cursor position */
  uint8_t y, x;

  /* Window location and size */
  uint8_t maxy, maxx;
  uint8_t begy, begx;
};


/* The TTY main-window. */
extern WINDOW tty_mainwin;
#define curscr (&tty_mainwin)

void initscr (void);
WINDOW *subwin (WINDOW *, uint8_t lines, uint8_t cols, uint8_t by, uint8_t bx);

/* Printing and cursor movement commands */
void waddch (WINDOW *, const char);
void waddstr (WINDOW *, const char *);
void waddstr_P (WINDOW *, const char *);
void wmove (WINDOW *, uint8_t y, uint8_t x);
void wprintw (WINDOW *, const char *, ...);
void wprintw_P (WINDOW *, const char *, ...);

#define addch(ch)		waddch(curscr,ch)
#define addstr(str)		waddstr(curscr,str)
#define addstr_P(str)		waddstr_P(curscr,str)
#define move(y,x)		wmove(curscr,y,x)
#define printw(str...)		wprintw(curscr,str)
#define printw_P(str...)	wprintw_P(curscr,str)

#define mvaddch(y,x,ch)		do { move(y,x); addch(ch); } while(0)
#define mvaddstr(y,x,str)	do { move(y,x); addstr(str); } while(0)
#define mvaddstr_P(y,x,str)	do { move(y,x); addstr_P(str); } while(0)
#define mvprintw(y,x,str...)	do { move(y,x); printw(str); } while(0)
#define mvprintw_P(y,x,str...)	do { move(y,x); printw_P(str); } while(0)

#define mvwaddch(w,ch)		do { wmove(w,y,x); waddch(w,ch); } while(0)
#define mvwaddstr(w,str)	do { wmove(w,y,x); waddstr(w,str); } while(0)
#define mvwaddstr_P(w,str)	do { wmove(w,y,x); waddstr_P(w,str); } while(0)
#define mvwprintw(w,y,x,str...)	do { wmove(w,y,x); wprintw(w,str); } while(0)
#define mvwprintw_P(w,y,x,str...)	\
  do { wmove(w,y,x); wprintw_P(w,str); } while(0)

/* Input */
char getch (void);
uint8_t _getch_queue (char);

/* Clearing, etc. */
void wclear (WINDOW *);
void wclrtobot (WINDOW *);
void wclrtoeol (WINDOW *);
void wscroll (WINDOW *, uint8_t);

#define clear()			wclear(curscr)
#define clrtobot()		wclrtobot(curscr)
#define clrtoeol()		wclrtoeol(curscr)
#define scroll(i)		wscroll(curscr,i)

/* Include Low-Level driver's Header File */
#ifdef TTY_LL_HD44780
#include "core/tty/tty-hd44780.h"
#endif
#ifdef TTY_LL_S1D15G10
#include "core/tty/tty-s1d15g10.h"
#endif
#ifdef TTY_LL_S1D13305
#include "core/tty/tty-s1d13305.h"
#endif

#ifdef TTY_LL_VT100_TELNET
#include "core/tty/tty-vt100.h"
#include "core/tty/tty-vt100-telnet.h"
#endif

#endif  /* TTY_H */
