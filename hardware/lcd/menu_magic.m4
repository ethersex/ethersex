dnl
dnl   Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 3 as
dnl   published by the Free Software Foundation.
dnl  
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl  
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl  
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl

#include <avr/pgmspace.h>
#include "hardware/lcd/menu.h"

#ifndef NULL
#define NULL ((void *) 0L)
#endif

divert(-1)
define(`prototypes', 0)
define(`definitions', 1)

define(`pushdivert', `define(`_old_divert', divnum)')
define(`popdivert', `divert(_old_divert)')

define(`pstrlabel', lcd_menu_text_$1)

define(`pstr', `dnl
pushdivert()dnl
divert(prototypes)const char PROGMEM pstrlabel(__line__)[] = $1;
popdivert() pstrlabel(__line__) dnl
')

define(`menu', `
divert(prototypes)struct lcd_menu_item lcd_menu_$1[];
divert(definitions)struct lcd_menu_item PROGMEM lcd_menu_$1[] = {')

define(`endmenu', `{ LCD_MENU_ITEM_LAST, NULL, NULL }
};')

define(`action', `{ LCD_MENU_ITEM_ACTION, pstr($1), $2 },')

define(`submenu', `{ LCD_MENU_ITEM_SUBMENU, pstr($1), lcd_menu_$2 },')

divert(definitions)
divert(0)