/*
*
* Copyright (c) 2009 by Jonny Roeker <dg9oaa@darc.de>
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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"

#include "hardware/tower/tower.h"


int16_t parse_cmd_pushbutton_up (char *cmd, char *output, uint16_t len)
{
   uint16_t time;
   uint8_t ret;

   while(*cmd == ' ') cmd++;

   ret = sscanf_P(cmd, PSTR("%d"), &time );
   if (ret == 1) {
     push_button_up(time);
     return ECMD_FINAL_OK;
   } else
     return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_pushbutton_down (char *cmd, char *output, uint16_t len)
{
   uint16_t time = 0;
   uint8_t ret  = 0;

   ret = sscanf_P(cmd, PSTR("%d"), &time );
   if (ret == 1) {
      push_button_down(time);
      return ECMD_FINAL_OK;
   } else
      return ECMD_ERR_PARSE_ERROR;
}

// depricated
int16_t parse_cmd_pushbutton (char *cmd, char *output, uint16_t len)
{
   uint16_t val1 = 0;
   uint16_t val2 = 0;
   uint8_t ret  = 0;

   ret = sscanf_P(cmd, PSTR("%hhu %hhu"), &val1, &val2 );
   if (ret == 2)
	   //return ECMD_FINAL_OK;
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%u:%u"), val1, val2 ));
   else
      return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_getstatus (char *cmd, char *output, uint16_t len)
{
   char* rtstr[] = {"np", "up", "down"};
   uint8_t ret;
   if (is_button_up == 1)
      ret =  1;
   else if (is_button_down == 1)
      ret =  2;
   else
      ret =  0;
   return ECMD_FINAL(snprintf_P(output, len, PSTR("%s -> %d"),rtstr[ret], timervalue));

}

int16_t parse_cmd_set_power (char *cmd, char *output, uint16_t len)
{
   //char value[10];
   uint8_t value;
   uint8_t ret  = 0;

   //ret = sscanf_P(cmd, PSTR("%s"), &value);
   ret = sscanf_P(cmd, PSTR("%u"), &value);
   
   if (ret == 1) {
      set_power(value);
      return ECMD_FINAL_OK;
   } else
      return ECMD_ERR_PARSE_ERROR;
}

//  ecmd_feature(pushbutton, "tower set", BUTTON TIME, push the button)

/*
  -- Ethersex META --
  header(hardware/tower/tower.h)
  timer(50, tower_periodic())
  init(tower_init)
  ecmd_feature(pushbutton_up, "tower up" , TIME, push the button up)
  ecmd_feature(pushbutton_down, "tower down", TIME, push the button down)
  ecmd_feature(set_power, "tower power", ON, switch power)
  ecmd_feature(getstatus, "tower status", , get the button)
*/

