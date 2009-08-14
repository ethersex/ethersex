/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "config.h"

#ifdef JOYSTICK_DIGITAL_SUPPORT

#include "joystick_digital.h"
#include "buttons.h"
#include "hardware/input/game_commons.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_JOYSTICK_DIGITAL
#include "core/debug.h"
#endif

void
joystick_digital_init(void)
{
  PIN_SET(BUTTON_UP);
  PIN_SET(BUTTON_DOWN);
  PIN_SET(BUTTON_LEFT);
  PIN_SET(BUTTON_RIGHT);
  PIN_SET(BUTTON_FIRE);
#ifdef DEBUG_GAME_INPUT
  hook_buttons_input_register(debug_game_input_handler);
#endif // DEBUG_GAME_INPUT
}

void
joystick_digital_periodic(void)
{
 if (PIN_HIGH(BUTTON_FIRE)==0){
   JOYSTICKDEBUG("FIRE");
   hook_buttons_input_call (HOOK_FIRE);
 }
 if (PIN_HIGH(BUTTON_UP)==0){
   JOYSTICKDEBUG("UP");
   hook_buttons_input_call (HOOK_UP);
 }
 if (PIN_HIGH(BUTTON_LEFT)==0){
   JOYSTICKDEBUG("LEFT");
   hook_buttons_input_call (HOOK_LEFT);
 }
 if (PIN_HIGH(BUTTON_RIGHT)==0){
   JOYSTICKDEBUG("RIGHT");
   hook_buttons_input_call (HOOK_RIGHT);
 }
 if (PIN_HIGH(BUTTON_DOWN)==0){
   JOYSTICKDEBUG("DOWN");
   hook_buttons_input_call (HOOK_DOWN);
 }
}

#endif // JOYSTICK_DIGITAL_SUPPORT

/*
  -- Ethersex META --
  header(hardware/input/buttons/joystick_digital.h)
  timer(10, joystick_digital_periodic())
  init(joystick_digital_init)
*/
