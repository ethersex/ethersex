/*
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

#ifndef BUTTONS_H
#define BUTTONS_H

#include "config.h"
#ifdef BUTTONS_INPUT_SUPPORT

#define BUTTON_NOPRESS 0
#define BUTTON_PRESS 1
#define BUTTON_LONGPRESS 2
#define BUTTON_REPEAT 3

typedef struct
{
  uint8_t status:2;
  uint8_t curStatus:1;
  uint8_t unused:5;
  uint8_t ctr;
}btn_statusType;

typedef struct
{
  const volatile uint8_t *port;
  const uint8_t pin;
}button_configType;

void
buttons_init(void);

void
buttons_periodic(void);

#ifdef DEBUG_BUTTONS_INPUT
# include "core/debug.h"
# define BUTTONDEBUG(a...)  debug_printf("button: " a)
#else
# define BUTTONDEBUG(a...)
#endif

#define HOOK_NAME btn_input
#define HOOK_ARGS (uint8_t btn, uint8_t status)
#include "hook.def"
#undef HOOK_NAME
#undef HOOK_ARGS

#endif  /* BUTTONS_INPUT_SUPPORT */
#endif  /* BUTTONS_H */
