/*
* Copyright (c) 2009 by Daniel Walter <fordprfkt@googlemail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
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

#ifndef _S1D13305_H
#define _S1D13305_H

#include <stdint.h>
#include <stdbool.h>

/**** S1D13305 CONTROL CODES ****/
/* System control */
#define CMD_SYSTEM_SET 0x40 /*Initialize device and display */
#define CMD_SLEEP_IN   0x53 /* Enter standby mode */
/* Display control */
#define CMD_DISP_OFF   0x58 /* Disable display */
#define CMD_DISP_ON    0x59 /* Enable display */
#define CMD_SCROLL     0x44 /* Display start address */
#define CMD_CSRFORM    0x5D /* Set cursor type */
#define CMD_CGRAMADR   0x5C /* Set CGRAM start address */
#define CMD_CSRDIR_RI  0x4C /* Cursor movement right */
#define CMD_CSRDIR_LE  0x4D /* Cursor movement left */
#define CMD_CSRDIR_UP  0x4E /* Cursor movement up */
#define CMD_CSRDIR_DN  0x4F /* Cursor movement down */
#define CMD_HDOT_SCR   0x5A /* Set horizontal scroll pos. */
#define CMD_OVLAY      0x5B /* Set display overlay format */
/* Drawing control */
#define CMD_CSRW       0x46 /* Set cursor address */
#define CMD_CSRR       0x47 /* Read Cursor address */
/* Memory control */
#define CMD_MWRITE     0x42 /* Write display memory */
#define CMD_MREAD      0x43 /* Read display memory */

#define LOW_BYTE(_v) (0xFF & _v)
#define HIGH_BYTE(_v) ((0xFF00 & _v) >> 8)

/* LCD cursor modes */
#define	LCD_CURSOR_OFF 0x00
#define	LCD_CURSOR_ON 0x01
#define	LCD_CURSOR_BLINK_SLOW 0x02
#define	LCD_CURSOR_BLINK_FAST 0x03

/* LCD screen modes */
#define	LCD_LAYER1_OFF 0x00
#define	LCD_LAYER1_ON 0x04
#define	LCD_LAYER1_BLINK_SLOW 0x08
#define	LCD_LAYER1_BLINK_FAST 0x0C
#define	LCD_LAYER2_OFF 0x00
#define	LCD_LAYER2_ON 0x10
#define	LCD_LAYER2_BLINK_SLOW 0x20
#define	LCD_LAYER2_BLINK_FAST 0x20
#define	LCD_LAYER3_OFF 0x00
#define	LCD_LAYER3_ON 0x40
#define	LCD_LAYER3_BLINK_SLOW 0x80
#define	LCD_LAYER3_BLINK_FAST 0xC0

/* LCD Layers */
#define	LCD_LAYER1 1
#define	LCD_LAYER2 2
#define	LCD_LAYER3 4
#define	LCD_ALL_LAYERS 7

/* Screen composition modes */
#define	LCD_COMBINE_OR 0x00
#define	LCD_COMBINE_XOR 0x01
#define	LCD_COMBINE_AND 0x02
#define	LCD_COMBINE_POR 0x03

/* prototypes */
void lcd_init();
void lcd_reset();
void lcd_clear(uint8_t layer_ui8);
void lcd_drawBitmap(uint8_t layer_ui8, uint16_t posX_ui16, uint16_t posY_ui16, uint16_t byteWidth_ui16, uint16_t height_ui16);
uint8_t lcd_getBMPData(uint16_t address_ui16);
void lcd_writeTextPos(uint8_t x_ui8, uint8_t y_ui8, uint8_t* text_pui8, uint16_t len_ui16);
void lcd_writeText(uint8_t* text_pui8, uint16_t len_ui16);
void lcd_setDisplayOn(bool displayOn_b);
void lcd_setLayerModes(uint8_t cursorMode_ui8, uint8_t layerModes_ui8);
void lcd_setOverlayMode(uint8_t mode_ui8);
void lcd_setCursorPos(uint8_t layer_ui8, uint16_t posX_ui16, uint16_t posY_ui16);
void lcd_writeData(uint8_t data_ui8);
uint8_t lcd_readData();
void lcd_writeCmdByte(uint8_t cmd_ui8);
void lcd_writeCmd(uint8_t cmd_ui8, uint8_t size_ui8, uint8_t* param_aui8);
void lcd_waitForCntrlrReady(void);
void lcd_setSleepMode(bool sleepIn_b);
void lcd_backlightOn(bool lightOn_b);

#ifdef DEBUG_S1D13305
# include "core/debug.h"
# define LCDDEBUG(a...)  debug_printf("LCD: " a)
#else
# define LCDDEBUG(a...)
#endif

#endif /* _S1D13305_H */

/* EOF */
