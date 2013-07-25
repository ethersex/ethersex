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

define(`MPC_USED', `ifdef(`mpc_used', `', `dnl
define(`mpc_used')dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
PGM_P mpc_command;

const char PROGMEM mpc_play[] = "play";
const char PROGMEM mpc_stop[] = "stop";
const char PROGMEM mpc_pause[] = "pause";
const char PROGMEM mpc_prev[] = "previous";
const char PROGMEM mpc_next[] = "next";

divert(old_divert)
    TCP_HANDLER_PERSIST(`mpc');
	for (;;) {
	    TCP_EXPECT("OK");
	    PT_WAIT_UNTIL(pt, mpc_command);
	    TCP_SEND("%S\n", mpc_command);
	    mpc_command = NULL;	    /* command packet was acked */
	}
    TCP_HANDLER_END();
')')

define(`MPC_CONNECT', `MPC_USED()TCP_CONNECT($1,$2,mpc)')

define(`MPC_PLAY', `MPC_USED()mpc_command = mpc_play;')
define(`MPC_STOP', `MPC_USED()mpc_command = mpc_stop;')
define(`MPC_PAUSE', `MPC_USED()mpc_command = mpc_pause;')
define(`MPC_PREV', `MPC_USED()mpc_command = mpc_prev;')
define(`MPC_NEXT', `MPC_USED()mpc_command = mpc_next;')


