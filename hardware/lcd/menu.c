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

#include "hardware/input/buttons/buttons.h"
#include "hd44780.h"
#include "menu.h"

static struct lcd_menu_item *active_menu;
static uint8_t active_menu_item;

#define LCD_MENU_BUTTON_PREV	0
#define LCD_MENU_BUTTON_ACTION	1
#define LCD_MENU_BUTTON_NEXT	2
#define LCD_MENU_BUTTON_CANCEL  3

static uint8_t
lcd_menu_get_active_type (void)
{
  struct lcd_menu_item i;
  memcpy_P (&i, &active_menu[active_menu_item], sizeof (struct lcd_menu_item));
  return i.type;
}


static void
lcd_menu_depict (void)
{
  MENUDEBUG ("Show menuitem %d of menu %p\n", active_menu_item, active_menu);

  struct lcd_menu_item i;
  memcpy_P (&i, &active_menu[active_menu_item], sizeof (struct lcd_menu_item));

  MENUDEBUG ("... type %d, label %S\n", i.type, i.label);

  switch (i.type)
    {
    case LCD_MENU_ITEM_SUBMENU:
    case LCD_MENU_ITEM_ACTION:
      hd44780_goto (1, 0);
      fprintf_P (lcd, PSTR ("%-16S"), i.label);
      break;

    case LCD_MENU_ITEM_LAST:
      (void) 0;			/* Do nothing, but keep compiler happy. */
    }
}


static void
lcd_menu_activate_menu (struct lcd_menu_item *m)
{
  MENUDEBUG ("activating menu %p\n", m);

  active_menu = m;
  active_menu_item = 0;

  lcd_menu_depict ();
}


static void
lcd_menu_button (uint8_t button)
{
  MENUDEBUG ("Button event: %d\n", button);

  if (button == LCD_MENU_BUTTON_ACTION)
    {
      if (active_menu == NULL)
	{
	  MENUDEBUG ("No menu active, activating root menu.\n");
	  lcd_menu_activate_menu (lcd_menu_root);
	}
      else
	{
	  struct lcd_menu_item i;
	  memcpy_P (&i, &active_menu[active_menu_item],
		    sizeof (struct lcd_menu_item));

	  if (i.data == NULL)
	    MENUDEBUG ("no handler assigned.  stop.\n");

	  else
	    switch (i.type)
	      {
	      case LCD_MENU_ITEM_SUBMENU:
		lcd_menu_activate_menu (i.data);
		break;

	      case LCD_MENU_ITEM_ACTION:
		break;

	      case LCD_MENU_ITEM_LAST:
		(void) 0;			/* Do nothing, but keep
						   compiler happy. */
	      }
	}
    }

  else if (button == LCD_MENU_BUTTON_PREV && active_menu)
    {
      if (active_menu_item == 0)
	{
	  MENUDEBUG ("seeking last menu item ...\n");
	  for (active_menu_item = 0; ; active_menu_item ++)
	    if (lcd_menu_get_active_type () == LCD_MENU_ITEM_LAST)
	      {
		active_menu_item --;
		break;
	      }
	}
      else
	active_menu_item --;

      lcd_menu_depict ();
    }

  else if (button == LCD_MENU_BUTTON_NEXT && active_menu)
    {
      active_menu_item ++;

      if (lcd_menu_get_active_type () == LCD_MENU_ITEM_LAST)
	active_menu_item = 0;

      lcd_menu_depict ();
    }

  else if (button == LCD_MENU_BUTTON_CANCEL)
    {
      hd44780_clear ();
      active_menu = NULL;
    }
}

void
lcd_menu_init (void)
{
  hook_buttons_input_register (lcd_menu_button);
}

/*
  -- Ethersex META --
  init(lcd_menu_init)
*/
