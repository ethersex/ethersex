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

#ifndef LCD_MENU_H
#define LCD_MENU_H 1

enum lcd_menu_item_type {
  LCD_MENU_ITEM_SUBMENU,
  LCD_MENU_ITEM_ACTION,
  LCD_MENU_ITEM_LAST,
};

struct lcd_menu_item {
  enum lcd_menu_item_type	type;
  PGM_P				label;
  void *			data;
};

extern struct lcd_menu_item lcd_menu_root[];

#ifdef DEBUG_LCD_MENU
# include "core/debug.h"
# define MENUDEBUG(a...)  debug_printf("menu: " a)
#else
# define MENUDEBUG(a...)
#endif

#endif	/* LCD_MENU_H */
