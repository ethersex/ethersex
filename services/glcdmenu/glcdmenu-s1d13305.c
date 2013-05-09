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

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "glcdmenu.h"
#include "glcdmenu-s1d13305.h"
#include "hardware/lcd/s1d13305/s1d13305.h"

#if S1D13305_MODE != S1D13305_GRAPHICS
#error "We need 3 Layer Graphics mode here."
#endif

/* Layer to be drawn on */
uint8_t drawLayer_ui8 = LCD_LAYER2;

/* Buffer for current display data */
uint8_t currDrawByte_ui8 = 0;

/* current display pos. */
uint16_t currDrawX_ui16 = UINT16_MAX;
uint16_t currDrawY_ui16 = UINT16_MAX;

/**
 * @brief Draws a pixel of the menu.
 *
 * Drawing is done directly into the video RAM. Since the
 * display memory is organized in bytes, we try to reduce
 * write cycles by buffering writes to the same RAM address
 * until a different address is written.
 *
 * @param xPos_ui16 X position on the screen
 * @param yPos_ui16 Y position on the screen
 * @param color_ui8 color of the pixel
 */
void glcdmenuDrawS1D13305(uint16_t xPos_ui16, uint16_t yPos_ui16,
		uint8_t color_ui8)
{
	if ((xPos_ui16 < CONF_S1D13305_RESX) && (yPos_ui16 < CONF_S1D13305_RESY))
	{
		/* Since we are drawing into a different byte now, send the current byte
		 * to the controller first */
		if ((xPos_ui16 / 8 != currDrawX_ui16 / 8) || (currDrawY_ui16
				!= yPos_ui16))
		{
			/* Write... */
			lcd_setCursorPos(drawLayer_ui8, currDrawX_ui16, currDrawY_ui16);
			lcd_writeCmdByte(CMD_MWRITE);
			lcd_writeData(currDrawByte_ui8);

			/* Read the initial contents of the new byte */
			lcd_setCursorPos(drawLayer_ui8, xPos_ui16, yPos_ui16);
			lcd_writeCmdByte(CMD_MREAD);
			currDrawByte_ui8 = lcd_readData();

			currDrawX_ui16 = xPos_ui16;
			currDrawY_ui16 = yPos_ui16;
		}

		/*  Draw */
		if (color_ui8 & 1)
		{
			currDrawByte_ui8 |= (0x80 >> (xPos_ui16 % 8));
		}
		else
		{
			currDrawByte_ui8 &= ~(0x80 >> (xPos_ui16 % 8));
		}
	}
	else
	{
		GLCDMENUDEBUG("Warning: Drawing %i, %i is out of bounds\n", xPos_ui16, yPos_ui16);
	}
}

/**
 * @brief Switches display layer
 *
 * When a display layer is drawn the current display layer
 * is made visible, and the new dar layer is made invisible.
 */
void glcdmenuFlushS1D13305(void)
{
	/* First write the current draw byte to the display */
	lcd_setCursorPos(drawLayer_ui8, currDrawX_ui16, currDrawY_ui16);
	lcd_writeCmdByte(CMD_MWRITE);
	lcd_waitForCntrlrReady();
	lcd_writeData(currDrawByte_ui8);

	currDrawByte_ui8 = 0;
	currDrawX_ui16 = UINT16_MAX;
	currDrawY_ui16 = UINT16_MAX;

	/* Switch layers */
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

/**
 * @brief Clears the current draw layer
 */
void glcdmenuClearS1D13305(void)
{
	lcd_clear(drawLayer_ui8);

}

/**
 * @brief Initializes the display
 */
void glcdmenuInitS1D13305(void)
{
	/* Menu uses layers 2+3 */
	lcd_setLayerModes(LCD_CURSOR_OFF, LCD_LAYER1_ON | LCD_LAYER2_OFF
			| LCD_LAYER3_OFF);
	lcd_setOverlayMode(LCD_COMBINE_OR);

	drawLayer_ui8 = LCD_LAYER2;
	currDrawByte_ui8 = 0;
	currDrawX_ui16 = UINT16_MAX;
	currDrawY_ui16 = UINT16_MAX;
}
