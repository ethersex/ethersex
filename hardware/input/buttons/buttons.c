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

/* Enable the hook buttons_input */
#define HOOK_NAME buttons_input
#define HOOK_ARGS (buttons_ButtonsType button, uint8_t status)
#define HOOK_COUNT 3
#define HOOK_ARGS_CALL (button, status)
#define HOOK_IMPLEMENT 1

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "buttons.h"
#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

/* This driver uses the E6 hook mechanism to notify of events. (see hook.def
 * for details)
 * To get notified in your application of a button press:
 * 1) #include "buttons.h"
 *
 * 2) Define a handler function in your application:
 *    void hook_button_handler(buttons_ButtonsType button, uint8_t status) {
 *      debug_printf("Button %d Status: %d\n",button, status);
 *    }
 *
 * 3) Register it for callback (in the Init Function of your app):
 *    hook_buttons_input_register(hook_buttons_handler);
 *
 *    Upon a button press, the function will be called with the following
 *    parameters:
 *    button           = The button that was pressed.
 *    status           = One of the following values:
 *      BUTTON_PRESS     = Button was pressed
 *                         <CONF_BUTTONS_DEBOUNCE_TIME> * 20ms (default: 80ms)
 *      BUTTON_LONGPRESS = Button was pressed for
 *                         <CONF_BUTTONS_LONGPRESS_TIME> * 20ms (default: 2s)
 *      BUTTON_REPEAT    = Button was pressed for
 *                         <CONF_BUTTONS_REPEAT_DELAY> * 20ms (default 3.5s),
 *                         this event is then repeated every
 *                         <CONF_BUTTONS_REPEAT_RATE> * 20ms (default 0.5s)
 *                         until the button is released.
 *      BUTTON_RELEASE   = Button released.
 */

#ifdef BUTTONS_INPUT_SUPPORT

#ifdef DEBUG_BUTTONS
  /*  For providing the actual name of the buttons for debug output */
  #define STR(_v)  const char _v##_str[] PROGMEM = #_v;
  #define STRLIST(_v) _v##_str,
  #define BUTTONS_GET_NAME(i) ((PGM_P)pgm_read_word(&buttonNames[i]))

  /* This creates an array of string in ROM which hold the button names. */
  BUTTONS_CONFIG(STR);
  PGM_P const buttonNames[BUTTONS_COUNT] PROGMEM = { BUTTONS_CONFIG(STRLIST) };
#endif

const buttons_configType buttonConfig[BUTTONS_COUNT] PROGMEM =
  { BUTTONS_CONFIG(C) };
buttons_statusType buttonStatus[BUTTONS_COUNT];

/**
 * @brief Initializes the module after startup
 *
 * Resets all buttons to BUTTON_RELEASE.
 *
 * @param void
 * @returns void
 */
void
buttons_init(void)
{
  BUTTONS_DEBUG("Init\n");

#ifdef CONF_BUTTONS_USE_PULLUPS
  BUTTONS_CONFIG(PULLUP);
#endif
}

/**
 * @brief Updates the button status every 20ms
 *
 * Reads in the button pins and runs the simple state machine for
 * short/longpress and repeat.
 *
 * @param void
 * @returns void
 */
void
buttons_periodic(void)
{
  /* Check all configured buttons */
  for (uint8_t i = 0; i < BUTTONS_COUNT; i++)
  {
    /* Get current value from portpin... */
#if CONF_BUTTONS_LEVEL == 2
    uint8_t curState =
      ((*((portPtrType) pgm_read_word(&buttonConfig[i].portIn)) &
      _BV(pgm_read_byte(&buttonConfig[i].pin))) ==
      _BV(pgm_read_byte(&buttonConfig[i].pin))) ? 0 : 1;
#else
    uint8_t curState =
      ((*((portPtrType) pgm_read_word(&buttonConfig[i].portIn)) &
      _BV(pgm_read_byte(&buttonConfig[i].pin))) ==
     _BV(pgm_read_byte(&buttonConfig[i].pin))) ? 1 : 0;
#endif

    /* Actual state hasn't change since the last read... */
    if (buttonStatus[i].curStatus == curState)
    {
      /* If the current button state is different from the last stable state,
       * run the debounce timer. Also keep the debounce timer running if the
       * button is pressed, because we need it for long press/repeat
       * recognition */
      if ((buttonStatus[i].curStatus != buttonStatus[i].status) ||
          (BUTTON_RELEASE != buttonStatus[i].status))
      {
        buttonStatus[i].ctr++;
      }
    }
    else
    {
      /* Actual state has changed since the last read. Restart the debounce
       * timer */
      buttonStatus[i].ctr = 0;
      buttonStatus[i].curStatus = curState;
    }

    /* Button was stable for DEBOUNCE_TIME * 20 ms */
    if (CONF_BUTTONS_DEBOUNCE_DELAY <= buttonStatus[i].ctr)
    {
      /* Button is pressed.. */
      if (1 == buttonStatus[i].curStatus)
      {
        switch (buttonStatus[i].status)
        {
          /* ..and was not pressed before. Send the PRESS event */
          case BUTTON_RELEASE:
            buttonStatus[i].status = BUTTON_PRESS;
            BUTTONS_DEBUG("Pressed %S\n", BUTTONS_GET_NAME(i));
            hook_buttons_input_call(i, buttonStatus[i].status);
            break;

          /* ..and was pressed before. Wait for long press. */
          case BUTTON_PRESS:
            if (CONF_BUTTONS_LONGPRESS_DELAY <= buttonStatus[i].ctr)
            {
              /* Long press time reached. Send LONGPRESS event. */
              buttonStatus[i].status = BUTTON_LONGPRESS;
              BUTTONS_DEBUG("Long press %S\n", BUTTONS_GET_NAME(i));
              hook_buttons_input_call(i, buttonStatus[i].status);
            }
            break;

#ifdef CONF_BUTTONS_USE_REPEAT
          /* ..and was long pressed before. Wait for repeat start. */
          case BUTTON_LONGPRESS:
            if (CONF_BUTTONS_REPEAT_DELAY <= buttonStatus[i].ctr)
            {
              /* Repeat time reached. Send first REPEAT event. */
              buttonStatus[i].status = BUTTON_REPEAT;
              BUTTONS_DEBUG("Repeat %S\n", BUTTONS_GET_NAME(i));
              hook_buttons_input_call(i, buttonStatus[i].status);
            }
            break;

          /* ..and is in repeat. Send cyclic REPEAT events. */
          case BUTTON_REPEAT:
            if (CONF_BUTTONS_REPEAT_DELAY + CONF_BUTTONS_REPEAT_RATE <=
                buttonStatus[i].ctr)
            {
              buttonStatus[i].status = BUTTON_REPEAT;
              buttonStatus[i].ctr = CONF_BUTTONS_REPEAT_DELAY;
              BUTTONS_DEBUG("Repeat %S\n", BUTTONS_GET_NAME(i));
              hook_buttons_input_call(i, buttonStatus[i].status);
            }
            break;

#else
          case BUTTON_LONGPRESS:
            /* Wait for button release */
            break;

#endif
          default:
            BUTTONS_DEBUG("Oops! Invalid state.\n");
            break;
        }
      }
      else
      {
        /* Button is not pressed anymore. Send RELEASE. */
        buttonStatus[i].status = BUTTON_RELEASE;
        BUTTONS_DEBUG("Released %S\n", BUTTONS_GET_NAME(i));
        buttonStatus[i].ctr = 0;
        hook_buttons_input_call(i, buttonStatus[i].status);
      }
    }
  }
}

/*
 * -- Ethersex META --
 * header(hardware/input/buttons/buttons.h)
 * timer(1, buttons_periodic())
 * init(buttons_init)
 */

#endif //BUTTONS_INPUT_SUPPORT
