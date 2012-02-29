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
#include "config.h"
#include "protocols/ecmd/ecmd-base.h"
#include "buttons.h"
#include "buttons_cfg.h"

#ifdef BUTTONS_INPUT_SUPPORT

#define BUTTON_DEBOUNCE_TIME 2		/* Debounce time in ethersex ticks (20ms) */
#define BUTTON_LONG_PRESS_TIME 45   /* Time for long press in ethersex ticks (20ms) */

#ifdef DEBUG_BUTTONS_INPUT
  const char* buttonNames[CONF_NUM_BUTTONS] = {BTN_CONFIG(S)};
#endif

const button_configType buttonConfig[CONF_NUM_BUTTONS] = {BTN_CONFIG(C)};
btn_statusType buttonStatus[CONF_NUM_BUTTONS];

/**
 * @brief Initializes the module after startup
 *
 * Resets all buttons to NOT_PRESSED.
 *
 * @param void
 * @returns void
 */
void buttons_init(void)
{
  uint8_t ctr;

  BUTTONDEBUG("Init\n");

  for (ctr=0; ctr<CONF_NUM_BUTTONS; ctr++)
  {
    BUTTONDEBUG("Button %s Port %i pin %i \n", buttonNames[ctr], buttonConfig[ctr].port, buttonConfig[ctr].pin);

    /* No button pressed */
    buttonStatus[ctr].curStatus = 0;
    buttonStatus[ctr].status = BUTTON_NOPRESS;
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
void buttons_periodic(void)
{
  uint8_t ctr;
  uint8_t curState;

  /* Check all configured buttons */
  for (ctr=0; ctr<CONF_NUM_BUTTONS; ctr++)
  {
	/* Get current value from portpin... */
    curState = ((*buttonConfig[ctr].port & _BV(buttonConfig[ctr].pin)) == _BV(buttonConfig[ctr].pin)) ? 0:1;

    /* Actual state hasn't change since the last read... */
    if (buttonStatus[ctr].curStatus == curState)
    {
      /* If the current button state is different from the last stable state,
       * run the debounce timer. Also keep the debounce timer running if the
       * button is pressed, because we need it for long press/repeat recognition */
      if ((buttonStatus[ctr].curStatus != buttonStatus[ctr].status) ||
          (BUTTON_NOPRESS != buttonStatus[ctr].status))
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
    if (BUTTON_DEBOUNCE_TIME <= buttonStatus[ctr].ctr)
    {
      /* Button is pressed.. */
      if (1 == buttonStatus[ctr].curStatus)
      {
        switch (buttonStatus[ctr].status)
        {
          /* ..and was not pressed before. Send the PRESS event */
          case BUTTON_NOPRESS:
            buttonStatus[ctr].status = BUTTON_PRESS;
            BUTTONDEBUG("Pressed %s\n", buttonNames[ctr]);
            hook_btn_input_call(ctr, buttonStatus[ctr].status);
          break;

          /* ..and was pressed before. Wait for long press. */
          case BUTTON_PRESS:
            if (BUTTON_LONG_PRESS_TIME <= buttonStatus[ctr].ctr)
            {
              /* Long press time reached. Send LONGPRESS event. */
              buttonStatus[ctr].status = BUTTON_LONGPRESS;
              BUTTONDEBUG("Long press %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
          break;

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
            if (CONF_BTN_REPEAT_TIME+CONF_BTN_REPEAT_RATE <= buttonStatus[ctr].ctr)
            {
              buttonStatus[ctr].status = BUTTON_REPEAT;
              buttonStatus[ctr].ctr = CONF_BTN_REPEAT_RATE;
              BUTTONDEBUG("Repeat %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
          break;

          default:
            BUTTONDEBUG("Oops! Invalid state.\n");
          break;
        }
      }
      else
      {
    	/* Button is not pressed anymore. Send NO_PRESS. */
        buttonStatus[ctr].status = BUTTON_NOPRESS;
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
