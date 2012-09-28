/*
 *
 * Copyright (c) 2012 by Daniel Walter <fordprfkt@googlemail.com>
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

#ifndef BUTTONS_H
#define BUTTONS_H

#include "config.h"
#ifdef BUTTONS_INPUT_SUPPORT

#if !defined(CONF_NUM_BUTTONS) || !defined(BTN_CONFIG)
#error Error in pinning configuration for buttons module. Check your pinning configuration.
#endif

/*
 * To configure additional buttons add the following to your pinning
 * configuration:
 *
 * pin(BTN_UP, PC2, INPUT)
 * pin(BTN_RIGHT, PC3, INPUT)
 * pin(BTN_DOWN, PC4, INPUT)
 * pin(BTN_LEFT, PC5, INPUT)
 * pin(BTN_FIRE, PD2, INPUT)
 * pin(BTN_FIRE2, PD2, INPUT)
 *
 * #define CONF_NUM_BUTTONS 6
 *
 * #define BTN_CONFIG(_x) \
 * _x(BTN_UP)\
 * _x(BTN_DOWN)\
 * _x(BTN_LEFT)\
 * _x(BTN_RIGHT)\
 * _x(BTN_FIRE)\
 * _x(BTN_FIRE2)
 */

#define BUTTON_RELEASE 0         /* Button is not pressed */
#define BUTTON_PRESS 1             /* Short press */
#define BUTTON_LONGPRESS 2   /* Long press */
#define BUTTON_REPEAT 3          /* Repeat function enabled, repeatedly triggered until button released */

/* These macros allow to use the the same configuration
 * macro (in buttons_cfg.h) to initialize the
 * btn_ButtonsType enum, the button_configType struct and set the pullups. */
#define E(_v) _v,
#define C(_v) {.portIn = &PIN_CHAR(_v##_PORT), .pin = _v##_PIN},
#define PULLUP(_v) PIN_SET(_v);

typedef volatile uint8_t * const portPtrType;

/* Enum used in the cyclic function to loop over all buttons */
typedef enum
{
  BTN_CONFIG(E)
}btn_ButtonsType;

/* Static configuration data for each button */
typedef struct
{
  portPtrType portIn;
  const uint8_t pin;
} button_configType;

/* Status information for each button */
typedef struct
{
  uint8_t status :2; /* One of the values RELEASE, PRESS, LONGPRESS... */
  uint8_t curStatus :1; /* Current pin value */
  uint8_t unused :5;
  uint8_t ctr; /* Debounce timer */
} btn_statusType;

void buttons_init(void);
void buttons_periodic(void);

#ifdef DEBUG_BUTTONS_INPUT
#include "core/debug.h"
#define BUTTONDEBUG(a...)  debug_printf("button: " a)
#else
#define BUTTONDEBUG(a...)
#endif

#define HOOK_NAME btn_input
#define HOOK_ARGS (btn_ButtonsType btn, uint8_t status)
#include "hook.def"
#undef HOOK_NAME
#undef HOOK_ARGS

#endif /* BUTTONS_INPUT_SUPPORT */
#endif /* BUTTONS_H */
