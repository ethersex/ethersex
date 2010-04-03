/*
*
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

#ifndef _S1D13305_CFG_H
#define _S1D13305_CFG_H

#include "config.h"

/* for menuconfig */
#define	S1D13305_MIXED 1
#define	S1D13305_GRAPHICS 2

#define LCD_CHR_SIZE_X 8        /* Horizontal size of one char. (Unit Bits) */

#if S1D13305_MODE == S1D13305_GRAPHICS
 #define LCD_CHR_SIZE_Y 1       /* Vertical size of one char. (Unit lines) */
#else
 #define LCD_CHR_SIZE_Y 8       /* Vertical size of one char. (Unit lines) */
#endif

/* Width of one screen line (bytes) */
#define LCD_BYTEWIDTH (CONF_S1D13305_RESX / LCD_CHR_SIZE_X)
/* Size of one graphic screen (bytes) */
#define LCD_BYTES_GRAPHIC ((CONF_S1D13305_RESX / 8) * CONF_S1D13305_RESY)
 /* Size of one text screen (bytes) */
#define LCD_BYTES_TEXT ((CONF_S1D13305_RESX / LCD_CHR_SIZE_X) * (CONF_S1D13305_RESY / LCD_CHR_SIZE_Y))

/* TC/R calculation:
 * xtal = 10MHz crystal
 * fRefresh = 75 Hz
 * TC/R = (((xtal / fRefresh) / LF) - 1) / 9
*/
#define LCD_CR (LCD_BYTEWIDTH - 1)      /* CR parameter for SystemSet */
#define LCD_LF (CONF_S1D13305_RESY - 1) /* LF parameter for SystemSet */
                                        /* TCR parameter for SystemSet */
#define LCD_TCR ((((CONF_S1D13305_XTAL / CONF_S1D13305_REFRESH) / LCD_LF) - 1) / 9)

#define LCD_SYSTEMSET_P1 0x30   /* 0 0 IV 1 W/S M2 M1 M0 */

#if S1D13305_MODE == S1D13305_GRAPHICS
  #define LCD_SAD2 (uint16_t)(LCD_BYTES_GRAPHIC)       /* Layer 2 start address in video RAM */
  #define LCD_SAD3 (uint16_t)((LCD_BYTES_GRAPHIC * 2)) /* Layer 3 start address in video RAM */
#else
  #define LCD_SAD2 (uint16_t)(LCD_BYTES_TEXT)          /* Layer 2 start address in video RAM */
  #define LCD_SAD3 0								   /* no layer 3 in mixed mode */
#endif

#endif /* _S1D13305_CFG_H */

/* EOF */
