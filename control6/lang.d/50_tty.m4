dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

divert(0)#include "core/tty/tty.h"
divert(-1)dnl

define(`TTY_USED', `ifdef(`tty_used', `', `dnl
define(`tty_used')dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef TTY_SUPPORT
#error Please define tty support
#endif

WINDOW *c6win = curscr;

divert(old_divert)')')

define(`TTY_CLEAR', `TTY_USED()wclear(c6win);')
define(`TTY_CLRTOEOL', `TTY_USED()wclrtoeol(c6win);')
define(`TTY_GOTO', `TTY_USED()wmove(c6win,$1,$2);')
define(`TTY_HOME', `TTY_USED()wmove(c6win,0,0);')

define(`TTY_WRITE', `TTY_USED()ifelse(`$#', 1,dnl
`waddstr_P(c6win,PSTR($1));',
`wprintw_P(c6win,PSTR($1), shift($@));')')
define(`TTY_WRITE_TIME', `CLOCK_USED()TTY_USED()wprintw(c6win,"%02d:%02d:%02d", CLOCK_HOUR(), CLOCK_MIN(), CLOCK_SEC());')

define(`TTY_CREATE_WINDOW_NOSEL', `TTY_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)WINDOW *c6win_$1;
divert(init_divert)c6win_$1 = subwin(curscr, $2, $3, $4, $5);
divert(old_divert)')

define(`TTY_CREATE_WINDOW', `dnl
TTY_CREATE_WINDOW_NOSEL($*)
TTY_SELECT($1)')


define(`TTY_SELECT', `TTY_USED()c6win = c6win_$1;')

define(`TTY_GETCH', `TTY_USED()getch()')
