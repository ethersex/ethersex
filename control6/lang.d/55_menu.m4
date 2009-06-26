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

dnl Print the message $1 to the ``parent line'' of the menu.  $1 is a PGM_P
dnl to the message.
define(`_MENU_PRINT_PARENT', `dnl
	TTY_SELECT(menu)
	TTY_CLEAR()
	TTY_USED()waddstr_P(c6win,$1);
')



define(`_MENU_DO_MAIN', `dnl
	TTY_SELECT(menu)
	TTY_GOTO(1,0)
	$*;
	TTY_CLRTOEOL()
')


dnl Print the message $1 to the ``main line'' of the menu.  $1 is a PGM_P
dnl to the message.
define(`_MENU_PRINT_MAIN', `_MENU_DO_MAIN(waddstr_P(c6win,$1))')


dnl Exit the menu.
define(`_MENU_EXIT', `dnl
	TTY_SELECT(menu)
	TTY_CLEAR()
	THREAD_EXIT()
')


dnl Menuitem helper macro.  Probably you want to derive your menuitem
dnl implementations from this one.
define(`_MENUITEM', `
define(`thismenu', `menu'__line__)thismenu:
	`_MENU_PRINT_MAIN($1)'

	/* Read byte from TTY input buffer */
	{ uint8_t ch;
	PT_WAIT_UNTIL(pt, (`ch = TTY_GETCH()'));

	if (ch == 107)		/* k -> up */
		goto prevmenu;	/* jump up to the previous item ... */

	else if (ch == 10) 	/* return -> action */
		{ $2 }

	else if (ch != 106)	/* invalid response -> ignore */
		goto thismenu;	/* jump to current item -> try next char */

	/* handled everything except ch == 106 --> fall through */
	}
#undef prevmenu
`#define prevmenu' thismenu
')


dnl ==========================================================================
dnl MENUITEM(NAME, ACTIONS)
dnl ==========================================================================
define(`MENUITEM', `_MENUITEM(PSTR ($1), $2
		``_MENU_EXIT()''
)')


dnl ==========================================================================
dnl SUBMENU(NAME, ITEMS)
dnl ==========================================================================
define(`SUBMENU', `dnl
define(`submenu', `submenu_name'__line__)dnl
`define(`old_divert', divnum)dnl
divert(globals_divert)dnl
	`const char PROGMEM' submenu[] = $1;
divert(old_divert)'dnl
	_MENUITEM(`submenu', `dnl
		`submenu_activate'__line__:
		_MENU_PRINT_PARENT(submenu)

		#undef prevmenu
		`#define prevmenu' `submenu_activate'__line__

		/* Items of sub-menu $1 */
		$2
		/* End of sub-menu $1 */

		goto prevmenu;
		#undef prevmenu
		`#define prevmenu' `menu'__line__
')')



dnl ==========================================================================
dnl MENUEDIT_BOOL(NAME, VARIABLE)
dnl ==========================================================================
define(`MENUEDIT_BOOL', `dnl
define(`editbool_mi', `editbool_name'__line__)dnl
`define(`old_divert', divnum)dnl
divert(globals_divert)
ifdef(`menuedit_bool_used', `', `dnl
const char PROGMEM menuedit_bool_enabled[] = "Enabled";
const char PROGMEM menuedit_bool_disabled[] = "Disabled";
uint8_t menuedit_bool_state;
define(`menuedit_bool_used')')
divert(globals_divert)dnl
	const char PROGMEM 'editbool_mi`[] = $1;
divert(old_divert)'dnl
	_MENUITEM(editbool_mi, `dnl
		menuedit_bool_state = $2;
		_MENU_PRINT_PARENT(editbool_mi)
		`boolthing_activate'__line__:
		_MENU_PRINT_MAIN(menuedit_bool_state ? menuedit_bool_enabled : menuedit_bool_disabled)

		/* Read byte from TTY input buffer */
		{ uint8_t ch;
		PT_WAIT_UNTIL(pt, (ch = TTY_GETCH()));

		if (ch == 107 || ch == 106) {	/* j or k -> toggle */
			menuedit_bool_state = menuedit_bool_state == 0;
			`goto boolthing_activate'__line__;
		}

		else if (ch == 10) 	/* return -> action */
			{ $2 = menuedit_bool_state; _MENU_EXIT() }

		/* invalid response -> ignore */
		`goto boolthing_activate'__line__;
		}
')')


dnl ==========================================================================
dnl MENUEDIT_INT(NAME, VARIABLE, MIN, MAX)
dnl ==========================================================================
define(`MENUEDIT_INT', `dnl
define(`editint_mi', `editint_name'__line__)dnl
`define(`old_divert', divnum)dnl
divert(globals_divert)
ifdef(`menuedit_int_used', `', `dnl
uint8_t menuedit_int_state;
define(`menuedit_int_used')')
divert(globals_divert)dnl
	const char PROGMEM 'editint_mi`[] = $1;
divert(old_divert)'dnl
	_MENUITEM(editint_mi, `dnl
		menuedit_int_state = $2;
		_MENU_PRINT_PARENT(editint_mi)
		`intthing_activate'__line__:
		_MENU_DO_MAIN(wprintw(c6win, "Value: %3u", menuedit_int_state))

		/* Read byte from TTY input buffer */
		{ uint8_t ch;
		PT_WAIT_UNTIL(pt, (ch = TTY_GETCH()));

		if (ch == 106 && menuedit_int_state < $4)
			menuedit_int_state ++;

		else if (ch == 107 && menuedit_int_state > $3)
			menuedit_int_state --;

		else if (ch == 10) 	/* return -> action */
			{ $2 = menuedit_int_state; _MENU_EXIT() }

		`goto intthing_activate'__line__;
		}
')')


dnl ==========================================================================
dnl MENU(WIDTH, Y, X, NAME, ITEMS)
dnl ==========================================================================
define(`MENU', `
define(`old_divert', divnum)dnl
divert(init_divert)dnl
        TTY_CREATE_WINDOW_NOSEL(menu, 2, $1, $2, $3)

divert(globals_divert)dnl
	`const char PROGMEM parent_menu_name'__line__[] = $4;
divert(old_divert)dnl

	THREAD(menu)
rootmenu:
#undef prevmenu
#define prevmenu rootmenu
	{
		_MENU_PRINT_PARENT(`parent_menu_name'__line__)

		/* Items of menu $4 */
		$5
		/* End of menu $4 */
		goto prevmenu;
	}
	THREAD_END(menu)
')


dnl ==========================================================================
dnl MENU_START()
dnl ==========================================================================
define(`MENU_START', `THREAD_START(menu)')


dnl ==========================================================================
dnl MENU_STARTED()
dnl ==========================================================================
define(`MENU_STARTED', `THREAD_STARTED(menu)')

