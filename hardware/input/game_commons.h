/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef _GAME_INPUT_H
#define _GAME_INPUT_H

// predefined button numbers
#define HOOK_UP     0
#define HOOK_DOWN   1
#define HOOK_LEFT   2
#define HOOK_RIGHT  3
#define HOOK_FIRE   4
#define HOOK_FIRE2  5
#define HOOK_FIRE3  6
#define HOOK_FIRE4  7

#ifdef DEBUG_GAME_INPUT
void
debug_game_input_handler(uint8_t b);
# include "core/debug.h"
# define GAMEINPUTDEBUG(a...)  debug_printf("gameinput: " a)
#else
# define GAMEINPUTDEBUG(a...)
#endif

#endif // _GAME_INPUT_H
