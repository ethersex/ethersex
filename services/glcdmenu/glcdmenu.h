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

#ifndef HAVE_GLCDMENU_H
#define HAVE_GLCDMENU_H

int16_t
glcdmenuInit(void);

int16_t
glcdmenuCheckRedraw(void);

void
glcdmenuRedraw(void);

void
glcdmenuKeypress(unsigned char key_uc);

#ifdef GLCDMENU_MOUSE_SUPPORT
void
glcdmenuMouseEvent(uint16_t xPos_ui16, uint16_t yPos_ui16, unsigned char button_uc);
#endif

void
glcdmenuSetString(uint16_t idx_ui16, unsigned char* ptr_pc);

void
glcdmenuSetChkBoxState(uint16_t idx_ui16, uint8_t state_ui8);

uint8_t
glcdmenuGetChkBoxState(uint16_t idx_ui16);

void
glcdmenuSetRadioBtnState(uint16_t idx_ui16, uint8_t state_ui8);

uint8_t
glcdmenuGetRadioBtnState(uint16_t idx_ui16);

void
glcdmenuSelectListItem(uint16_t idx_ui16, uint16_t state_ui16);

uint16_t
glcdmenuGetListItem(uint16_t idx_ui16);

void
glcdmenuSetGfxData(uint16_t idx_ui16, unsigned char* ptr_pc);

#define HOOK_NAME menu_action
#define HOOK_ARGS (uint8_t menuAction)
#include "hook.def"
#undef HOOK_NAME
#undef HOOK_ARGS

#include "config.h"
#ifdef DEBUG_GLCD_MENU
# include "core/debug.h"
# define GLCDMENUDEBUG(a...)  debug_printf("glcdMenu: " a)
#else
# define GLCDMENUDEBUG(a...)
#endif

#endif  /* HAVE_GLCDMENU_H */
