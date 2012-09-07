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

/* Enable the hook btn_input */
#define HOOK_NAME btn_input
#define HOOK_ARGS (btn_ButtonsType btn, uint8_t status)
#define HOOK_COUNT 3
#define HOOK_ARGS_CALL (btn, status)
#define HOOK_IMPLEMENT 1

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "buttons.h"
#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

/* This driver uses the E6 hook mechanism to notify of events. (see hook.def for details)
 * To get notified in your application of a button press:
 * 1) #include "buttons.h"
 *
 * 2) Define a handler function in your application:
 *     void hook_btn_handler(btn_ButtonsType btn, uint8_t status) {
 *       debug_printf("Button %d Status: %d\n",btn, status);
 *     }
 *
 * 3) Then register it for callback (in the Init Function of your app):
 *     hook_btn_input_register(hook_btn_handler);
 *
 *     Upon a button press, the function will be called with the following parameters:
 *     btn = The Button that was pressed.
 *     status = One of the following values (BUTTON_PRESS, BUTTON_LONGPRESS, BUTTON_REPEAT, BUTTON_RELEASE)
 *
 *     BUTTON_PRESS 	= 	Button was pressed.
 *     BUTTON_LONGPRESS =	Button was pressed for 900ms
 *     BUTTON_REPEAT 	= 	Button was pressed for <CONF_BTN_REPEAT_TIME>ms, this event is then
 *                     		repeated every <CONF_BTN_REPEAT_RATE>ms until the button is released.
 *     BUTTON_RELEASE 	=	Button released.
 *
 */

#ifdef BUTTONS_INPUT_SUPPORT

#ifdef DEBUG_BUTTONS_INPUT
const char *buttonNames[CONF_NUM_BUTTONS] = { BTN_CONFIG(S) };
#endif

const button_configType buttonConfig[CONF_NUM_BUTTONS] = { BTN_CONFIG(C) };

btn_statusType buttonStatus[CONF_NUM_BUTTONS];

/**
 * @brief Initializes the module after startup
 *
 * Resets all buttons to NOT_PRESSED.
 *
 * @param void
 * @returns void
 */
void
buttons_init(void)
{
  uint8_t ctr;

  BUTTONDEBUG("Init\n");

#ifdef CONF_BTN_USE_PULLUPS
  BTN_CONFIG(PULLUP);
#endif

  for (ctr = 0; ctr < CONF_NUM_BUTTONS; ctr++)
  {
    BUTTONDEBUG("Button %s Port %i pin %i \n", buttonNames[ctr],
                buttonConfig[ctr].portIn, buttonConfig[ctr].pin);

    /* No button pressed */
    buttonStatus[ctr].curStatus = 0;
    buttonStatus[ctr].status = BUTTON_RELEASE;
    buttonStatus[ctr].ctr = 0;
  }
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
  uint8_t ctr;
  uint8_t curState;

  /* Check all configured buttons */
  for (ctr = 0; ctr < CONF_NUM_BUTTONS; ctr++)
  {
    /* Get current value from portpin... */
#if CONF_BUTTON_LVL == 2
    curState =
      ((*buttonConfig[ctr].portIn & _BV(buttonConfig[ctr].pin)) ==
       _BV(buttonConfig[ctr].pin)) ? 0 : 1;
#else
    curState =
      ((*buttonConfig[ctr].portIn & _BV(buttonConfig[ctr].pin)) ==
       _BV(buttonConfig[ctr].pin)) ? 1 : 0;
#endif
    /* Actual state hasn't change since the last read... */
    if (buttonStatus[ctr].curStatus == curState)
    {
      /* If the current button state is different from the last stable state,
       * run the debounce timer. Also keep the debounce timer running if the
       * button is pressed, because we need it for long press/repeat recognition */
      if ((buttonStatus[ctr].curStatus != buttonStatus[ctr].status) ||
          (BUTTON_RELEASE != buttonStatus[ctr].status))
      {
        buttonStatus[ctr].ctr++;
      }
    }
    else
    {
      /* Actual state has changed since the last read. Restart the debounce timer */
      buttonStatus[ctr].ctr = 0;
      buttonStatus[ctr].curStatus = curState;
    }

    /* Button was stable for DEBOUNCE_TIME*20 ms */
    if (CONF_BTN_DEBOUNCE_TIME <= buttonStatus[ctr].ctr)
    {
      /* Button is pressed.. */
      if (1 == buttonStatus[ctr].curStatus)
      {
        switch (buttonStatus[ctr].status)
        {
            /* ..and was not pressed before. Send the PRESS event */
          case BUTTON_RELEASE:
            buttonStatus[ctr].status = BUTTON_PRESS;
            BUTTONDEBUG("Pressed %s\n", buttonNames[ctr]);
            hook_btn_input_call(ctr, buttonStatus[ctr].status);
            break;

            /* ..and was pressed before. Wait for long press. */
          case BUTTON_PRESS:
            if (CONF_BTN_LONGPRESS_TIME <= buttonStatus[ctr].ctr)
            {
              /* Long press time reached. Send LONGPRESS event. */
              buttonStatus[ctr].status = BUTTON_LONGPRESS;
              BUTTONDEBUG("Long press %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
            break;

#ifdef CONF_BTN_USE_REPEAT
            /* ..and was long pressed before. Wait for repeat start. */
          case BUTTON_LONGPRESS:
            if (CONF_BTN_REPEAT_TIME <= buttonStatus[ctr].ctr)
            {
              /* Repeat time reached. Send REPEAT event. */
              buttonStatus[ctr].status = BUTTON_REPEAT;
              BUTTONDEBUG("Repeat %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
            break;

            /* ..and is in repeat. Send cyclic events. */
          case BUTTON_REPEAT:
            if (CONF_BTN_REPEAT_TIME + CONF_BTN_REPEAT_RATE <=
                buttonStatus[ctr].ctr)
            {
              buttonStatus[ctr].status = BUTTON_REPEAT;
              buttonStatus[ctr].ctr = CONF_BTN_REPEAT_RATE;
              BUTTONDEBUG("Repeat %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
            break;
#else
          case BUTTON_LONGPRESS:
        	  /* Wait for button release */
              break;
#endif

          default:
            BUTTONDEBUG("Oops! Invalid state.\n");
            break;
        }
      }
      else
      {
        /* Button is not pressed anymore. Send RELEASE. */
        buttonStatus[ctr].status = BUTTON_RELEASE;
        BUTTONDEBUG("Released %s\n", buttonNames[ctr]);
        buttonStatus[ctr].ctr = 0;
        hook_btn_input_call(ctr, buttonStatus[ctr].status);
      }
    }
  }
}

/*
  -- Ethersex META --
  header(hardware/input/buttons/buttons.h)
  timer(1, buttons_periodic())
  init(buttons_init)
*/

#endif //BUTTONS_INPUT_SUPPORT
