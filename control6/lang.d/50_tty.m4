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
define(`old_divert', divnum)dnl
define(`tty_used')dnl
divert(globals_divert)
#ifndef TTY_SUPPORT
#error Please define tty support
#endif

divert(old_divert)')')

define(`TTY_CLEAR', TTY_USED()clear();)
define(`TTY_GOTO', TTY_USED()move($1,$2);)
define(`TTY_WRITE', TTY_USED()addstr_P(PSTR($1));)

define(`TTY_WRITE_TIME', TTY_USED()printw("%02d:%02d:%02d", CLOCK_HOUR(), CLOCK_MIN(), CLOCK_SEC());)
