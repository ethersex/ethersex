/*
 * Copyright (c) 2011 by Daniel Walter <fordprfkt@googlemail.com>
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

#define BUTTON_DEBOUNCE_TIME 2
#define BUTTON_LONG_PRESS_TIME 45
#define BUTTON_REPEAT_START_TIME 50
#define BUTTON_REPEAT_TIME 10

#ifdef DEBUG_BUTTONS_INPUT
  const char* buttonNames[CONF_NUM_BUTTONS] = {BTN_CONFIG(S)};
#endif

const button_configType buttonConfig[CONF_NUM_BUTTONS] = {BTN_CONFIG(C)};
btn_statusType buttonStatus[CONF_NUM_BUTTONS];

void buttons_init(void)
{
  uint8_t ctr;

  BUTTONDEBUG("Init\n");

  for (ctr=0; ctr<CONF_NUM_BUTTONS; ctr++)
  {
    BUTTONDEBUG("Button %s Port %i pin %i \n", buttonNames[ctr], buttonConfig[ctr].port, buttonConfig[ctr].pin);
    buttonStatus[ctr].curStatus = 0;
    buttonStatus[ctr].status = BUTTON_NOPRESS;
    buttonStatus[ctr].ctr = 0;
  }
}

void buttons_periodic(void)
{
  uint8_t ctr;
  uint8_t curState;

  for (ctr=0; ctr<CONF_NUM_BUTTONS; ctr++)
  {
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

    if (BUTTON_DEBOUNCE_TIME <= buttonStatus[ctr].ctr)
    {
      if (1 == buttonStatus[ctr].curStatus)
      {
        switch (buttonStatus[ctr].status)
        {
          case BUTTON_NOPRESS:
            buttonStatus[ctr].status = BUTTON_PRESS;
            BUTTONDEBUG("Pressed %s\n", buttonNames[ctr]);
            hook_btn_input_call(ctr, buttonStatus[ctr].status);
          break;

          case BUTTON_PRESS:
            if (BUTTON_LONG_PRESS_TIME <= buttonStatus[ctr].ctr)
            {
              buttonStatus[ctr].status = BUTTON_LONGPRESS;
              BUTTONDEBUG("Long press %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
          break;

          case BUTTON_LONGPRESS:
            if (BUTTON_REPEAT_START_TIME <= buttonStatus[ctr].ctr)
            {
              buttonStatus[ctr].status = BUTTON_REPEAT;
              BUTTONDEBUG("Repeat %s\n", buttonNames[ctr]);
              hook_btn_input_call(ctr, buttonStatus[ctr].status);
            }
          break;

          case BUTTON_REPEAT:
            if (BUTTON_REPEAT_START_TIME+BUTTON_REPEAT_TIME <= buttonStatus[ctr].ctr)
            {
              buttonStatus[ctr].status = BUTTON_REPEAT;
              buttonStatus[ctr].ctr = BUTTON_REPEAT_START_TIME;
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
