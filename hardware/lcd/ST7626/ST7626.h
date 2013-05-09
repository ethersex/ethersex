/*
* Copyright (c) 2009 by Bastian Nagel <slime@voiceteam.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _ST7626_H
#define _ST7626_H

#include <stdio.h>
#include "config.h"

#ifdef ST7626_SUPPORT

//#define ST7626_DATA	PORTA

/* prototypes */
void ST7626_reset(void);
void ST7626_write(uint8_t mode, uint8_t data);
void ST7626_load_eeprom(void);
void ST7626_load_paint(void);
void ST7626_init(void);
void ST7626_clear(uint8_t color);
int ST7626_putc(char d, FILE *stream);
void ST7626_putpixel(uint8_t x, uint8_t y, uint16_t color);

#endif /* ST7626_SUPPORT */

#endif /* _ST7626_H */
