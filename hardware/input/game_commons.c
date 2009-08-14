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

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "config.h"

#ifdef GAME_INPUT_SUPPORT

#include "hardware/input/buttons/buttons.h"
#include "game_commons.h"

#ifdef DEBUG_GAME_INPUT
#include "core/debug.h"
void
debug_game_input_handler(uint8_t b)
{
   GAMEINPUTDEBUG("handle: %d\n", b);
}
#endif

#endif // GAME_INPUT_SUPPORT

/*
  -- Ethersex META --
*/
