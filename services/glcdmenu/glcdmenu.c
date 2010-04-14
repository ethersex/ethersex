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

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "glcdmenu.h"

#ifdef GLCDMENU_S1D13305
#include "hardware/lcd/s1d13305/s1d13305.h"
#if S1D13305_MODE != S1D13305_GRAPHICS
#error "We need 3 Layer Graphics mode here."
#endif
#else
#error "Your Display type is not supported at the moment. Sorry."
#endif /* GLCDMENU_S1D13305 */

#include "protocols/ecmd/ecmd-base.h"

#include "menu-interpreter.h"

#include "menudata-progmem.c"

/* Layer to be drawn on */
uint8_t drawLayer_ui8 = LCD_LAYER2;

/* true = redraw the menu */
bool doRedraw_b = true;

uint8_t currDrawByte_ui8 = 0;
uint16_t currDrawX_ui16 = UINT16_MAX;
uint16_t currDrawY_ui16 = UINT16_MAX;

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
 * menu. It is drwan directly into the video RAM. Since the
 * display memory is organized in bytes, we try to reduce
 * write cycles by buffering writes to the same RAM address
 * until a different address is written.
 *
 * @param x X position on the screen
 * @param y Y position on the screen
 * @param color color of the pixel
 */
void menu_screen_set(SCREENPOS x, SCREENPOS y, unsigned char color)
{
	if ((x < CONF_S1D13305_RESX) && (y < CONF_S1D13305_RESY))
	{
		if ((x / 8 != currDrawX_ui16 / 8) || (currDrawY_ui16 != y))
		{
			lcd_setCursorPos(drawLayer_ui8, currDrawX_ui16, currDrawY_ui16);
			lcd_writeCmdByte(CMD_MWRITE);
			lcd_writeData(currDrawByte_ui8);

			lcd_setCursorPos(drawLayer_ui8, x, y);
			lcd_writeCmdByte(CMD_MREAD);
			currDrawByte_ui8 = lcd_readData();

			currDrawX_ui16 = x;
			currDrawY_ui16 = y;
		}

		if (color & 1)
		{
			currDrawByte_ui8 |= (0x80 >> (x % 8));
		}
		else
		{
			currDrawByte_ui8 &= ~(0x80 >> (x % 8));
		}
	}
	else
	{
		GLCDMENUDEBUG("Warning: Drawing %i, %i is out of bounds\n", x, y);
	}
}

void menu_screen_flush(void)
{
	lcd_setCursorPos(drawLayer_ui8, currDrawX_ui16, currDrawY_ui16);
	lcd_writeCmdByte(CMD_MWRITE);
	lcd_waitForCntrlrReady();
	lcd_writeData(currDrawByte_ui8);

	currDrawByte_ui8 = 0;
	currDrawX_ui16 = UINT16_MAX;
	currDrawY_ui16 = UINT16_MAX;

	if (LCD_LAYER2 == drawLayer_ui8)
	{
		lcd_setLayerModes(LCD_CURSOR_OFF, LCD_LAYER1_ON | LCD_LAYER2_ON
				| LCD_LAYER3_OFF);
		drawLayer_ui8 = LCD_LAYER3;
	}
	else if (LCD_LAYER3 == drawLayer_ui8)
	{
		lcd_setLayerModes(LCD_CURSOR_OFF, LCD_LAYER1_ON | LCD_LAYER2_OFF
				| LCD_LAYER3_ON);
		drawLayer_ui8 = LCD_LAYER2;
	}
	else
	{
		GLCDMENUDEBUG("Could not switch draw layer\n");
	}
}

void menu_screen_clear(void)
{
	lcd_clear(drawLayer_ui8);

}

unsigned char menu_action(unsigned short action)
{
	GLCDMENUDEBUG("action: %i\n", action);
	return 0;
}

void glcdmenuRedraw(void)
{
	doRedraw_b = true;
}

void glcdmenuSetString(uint16_t idx_ui16, unsigned char* ptr_pc)
{
	menu_strings[idx_ui16] = ptr_pc;
}

void glcdmenuSetChkBoxState(uint16_t idx_ui16, uint8_t state_ui8)
{
	menu_checkboxstate[idx_ui16] = state_ui8;
}

void glcdmenuSetRadioBtnState(uint16_t idx_ui16, uint8_t state_ui8)
{
	menu_radiobuttonstate[idx_ui16] = state_ui8;
}

void glcdmenuSetListIndex(uint16_t idx_ui16, uint16_t state_ui16)
{
	menu_listindexstate[idx_ui16] = state_ui16;
}

void glcdmenuSetGfxData(uint16_t idx_ui16, unsigned char* ptr_pc)
{
	menu_gfxdata[idx_ui16] = ptr_pc;
}

int16_t glcdmenuCheckRedraw(void)
{
	if (true == doRedraw_b)
	{
		GLCDMENUDEBUG("Redraw\n");
		menu_redraw();
		doRedraw_b = false;
	}

	return ECMD_FINAL_OK;
}

int16_t glcdmenuInit(void)
{
	GLCDMENUDEBUG("init\n");

	lcd_setLayerModes(LCD_CURSOR_OFF, LCD_LAYER1_ON | LCD_LAYER2_OFF
			| LCD_LAYER3_OFF);
	lcd_setOverlayMode(LCD_COMBINE_OR);

	drawLayer_ui8 = LCD_LAYER2;
	currDrawByte_ui8 = 0;
	currDrawX_ui16 = UINT16_MAX;
	currDrawY_ui16 = UINT16_MAX;

	return ECMD_FINAL_OK;
}

/*
 -- Ethersex META --
 header(services/glcdmenu/glcdmenu.h)
 init(glcdmenuInit)
 timer(100,glcdmenuCheckRedraw())
 */
