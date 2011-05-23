/*
 * Copyright (c) 2010 by Daniel Walter <fordprfkt@googlemail.com>
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

#define HOOK_NAME menu_action
#define HOOK_ARGS (uint8_t menuAction)
#define HOOK_COUNT 2
#define HOOK_ARGS_CALL (menuAction)
#define HOOK_IMPLEMENT 1

#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "glcdmenu.h"
#include "protocols/ecmd/ecmd-base.h"

/* To satisfy the watchdog during lengthy redraws */
#undef MENU_SCHEDULE
#define MENU_SCHEDULE wdt_kick();

#ifdef GLCDMENU_MOUSE_SUPPORT
#define MENU_MOUSE_SUPPORT
#endif
#include "menu-interpreter/menu-interpreter.h"

#ifdef GLCDMENU_S1D13305
#include "glcdmenu-s1d13305.h"
#endif

#include "menu-interpreter/menudata-progmem.c"

/* true = redraw the menu */
bool doRedraw_b = true;

/**
 * @brief Returns one byte of the menu data from ROM
 *
 * Called from menu_interpreter.
 * Reads the menu data given in addr.
 *
 * @param addr Address offset to be read
 */
unsigned char menu_byte_get(MENUADDR addr)
{
	if (addr >= MENU_DATASIZE)
	{
		GLCDMENUDEBUG("Error: %i out of range\n", addr);
	}

	return pgm_read_byte(menudata + addr);
}

/**
 * @brief Draws a pixel of the menu.
 *
 * Called from menu_interpreter.
 * The menu_interpreter calls this function to draw the
 * menu.
 *
 * @param x X position on the screen
 * @param y Y position on the screen
 * @param color color of the pixel
 */
void menu_screen_set(SCREENPOS x, SCREENPOS y, unsigned char color)
{
#ifdef GLCDMENU_S1D13305
	glcdmenuDrawS1D13305(x, y, color);
#endif
}

/**
 * @brief Screen drawing is done.
 *
 * Called from menu_interpreter.
 * The menu_interpreter calls this function as
 * a screen is completely drawn.
 */
void menu_screen_flush(void)
{
#ifdef GLCDMENU_S1D13305
	glcdmenuFlushS1D13305();
#endif
}

/**
 * @brief Clears the screen.
 *
 * Called from menu_interpreter.
 * The menu_interpreter calls this function to
 * clear screen contents.
 */
void menu_screen_clear(void)
{
#ifdef GLCDMENU_S1D13305
	glcdmenuClearS1D13305();
#endif
}

/**
 * @brief React on a menu event (Button press, etc.)
 *
 * Called from menu_interpreter.
 * The menu_interpreter calls this function if a menu
 * event occurs. A user app can register a hook
 * to get notified of the event. See hook.def for details.
 */
unsigned char menu_action(unsigned short action)
{
	GLCDMENUDEBUG("action: %i\n", action);

	hook_menu_action_call(action);

	return 0;
}

/**
 * @brief Demand a redraw.
 *
 * Call this function to redraw the screen contents.
 */
void glcdmenuRedraw(void)
{
	doRedraw_b = true;
}

/**
 * @brief Notify the menu of a keypress.
 *
 * This function is quite lame because it just calls the
 * appropriate function of the menu interpreter but i
 * wanted to keep the interface clean.
 *
 * @param key_uc The Scancode of the key
 */
void glcdmenuKeypress(unsigned char key_uc)
{
	menu_keypress(key_uc);
}

#ifdef GLCDMENU_MOUSE_SUPPORT
/**
 * @brief Notify the menu of a mouse action.
 *
 * This function is quite lame because it just calls the
 * appropriate function of the menu interpreter but i
 * wanted to keep the interface clean.
 *
 * @param xPos_ui16 Mouse X-Pos
 * @param yPos_ui16 Mouse Y-Pos
 * @param button_uc The mouse button pressed (or scrollwheel)
 */
void glcdmenuMouseEvent(uint16_t xPos_ui16, uint16_t yPos_ui16, unsigned char button_uc)
{
	menu_mouse((SCREENPOS)xPos_ui16, (SCREENPOS)yPos_ui16, button_uc);
}
#endif

/**
 * @brief Set dynamic string contents.
 *
 * Call this function to set the contents of a dynamic string.
 * @param idx_ui16 Identifier of the label
 * @param ptr_pc Pointer to a string
 */
void glcdmenuSetString(uint16_t idx_ui16, unsigned char* ptr_pc)
{
	if (MENU_TEXT_MAX > idx_ui16)
	{
		menu_strings[idx_ui16] = ptr_pc;
	}
	else
	{
		GLCDMENUDEBUG("String index out of bounds: %i\n", idx_ui16);
	}
}

/**
 * @brief Set checkbox state.
 *
 * Call this function to set a checkbox state.
 * @param idx_ui16 Identifier of the checkbox
 * @param state_ui8 0 = unchecked, 1 = checked
 */
void glcdmenuSetChkBoxState(uint16_t idx_ui16, uint8_t state_ui8)
{
	if (MENU_CHECKBOX_MAX > idx_ui16)
	{
		menu_checkboxstate[idx_ui16] = state_ui8;
	}
	else
	{
		GLCDMENUDEBUG("ChkBox index out of bounds: %i\n", idx_ui16);
	}
}

/**
 * @brief Get checkbox state.
 *
 * Call this function to get a checkbox state.
 * @param idx_ui16 Identifier of the checkbox
 */
uint8_t glcdmenuGetChkBoxState(uint16_t idx_ui16)
{
	if (MENU_CHECKBOX_MAX > idx_ui16)
	{
		return menu_checkboxstate[idx_ui16];
	}
	else
	{
		GLCDMENUDEBUG("ChkBox index out of bounds: %i\n", idx_ui16);
		return UINT8_MAX;
	}
}

/**
 * @brief Set radio button state.
 *
 * Call this function to set a radio button state.
 * @param idx_ui16 Identifier of the radio button
 * @param state_ui8 0 = unchecked, 1 = checked
 */
void glcdmenuSetRadioBtnState(uint16_t idx_ui16, uint8_t state_ui8)
{
	if (MENU_RADIOBUTTON_MAX > idx_ui16)
	{
		menu_radiobuttonstate[idx_ui16] = state_ui8;
	}
	else
	{
		GLCDMENUDEBUG("RadioBtn index out of bounds: %i\n", idx_ui16);
	}
}

/**
 * @brief Get radio button state.
 *
 * Call this function to get a radio button state.
 * @param idx_ui16 Identifier of the radio button
 */
uint8_t glcdmenuGetRadioBtnState(uint16_t idx_ui16)
{
	if (MENU_RADIOBUTTON_MAX > idx_ui16)
	{
		return menu_radiobuttonstate[idx_ui16];
	}
	else
	{
		GLCDMENUDEBUG("RadioBtn index out of bounds: %i\n", idx_ui16);
		return UINT8_MAX;
	}
}

/**
 * @brief Select a list item.
 *
 * Call this function to select a item in a list.
 * @param idx_ui16 Identifier of the list
 * @param item_ui16 Item to select
 */
void glcdmenuSelectListItem(uint16_t idx_ui16, uint16_t item_ui16)
{
	if (MENU_LIST_MAX > idx_ui16)
	{
		menu_listindexstate[idx_ui16] = item_ui16;
	}
	else
	{
		GLCDMENUDEBUG("List index out of bounds: %i\n", idx_ui16);
	}
}

/**
 * @brief Get the selected list item.
 *
 * Call this function to return the selected list item.
 * @param idx_ui16 Identifier of the list
 */
uint16_t glcdmenuGetListItem(uint16_t idx_ui16)
{
	if (MENU_LIST_MAX > idx_ui16)
	{
		return menu_listindexstate[idx_ui16];
	}
	else
	{
		GLCDMENUDEBUG("List index out of bounds: %i\n", idx_ui16);
		return UINT16_MAX;
	}
}

/**
 * @brief Select a bitmap.
 *
 * Call this function to set the data pointer of a dynamic bitmap.
 * @param idx_ui16 Identifier of the bitmap
 * @param ptr_pc Pointer to bitmap data
 */
void glcdmenuSetGfxData(uint16_t idx_ui16, unsigned char* ptr_pc)
{
	if (MENU_GFX_MAX > idx_ui16)
	{
		menu_gfxdata[idx_ui16] = ptr_pc;
	}
	else
	{
		GLCDMENUDEBUG("GFX index out of bounds: %i\n", idx_ui16);
	}
}

/**
 * @brief Check if a redraw should be done.
 *
 * Redraws the menu if necessary.
 */
int16_t glcdmenuCheckRedraw(void)
{
	/* Check if a redraw should be done */
	if (true == doRedraw_b)
	{
		GLCDMENUDEBUG("Redraw\n");
		menu_redraw();
		doRedraw_b = false;
	}

	return ECMD_FINAL_OK;
}

/**
 * @brief Initialize the menu
 *
 * Initialize the menu app.
 */
int16_t glcdmenuInit(void)
{
	GLCDMENUDEBUG("init\n");

#ifdef GLCDMENU_S1D13305
	glcdmenuInitS1D13305();
#endif

	return ECMD_FINAL_OK;
}

/*
 -- Ethersex META --
 header(services/glcdmenu/glcdmenu.h)
 init(glcdmenuInit)
 timer(100,glcdmenuCheckRedraw())
 */
