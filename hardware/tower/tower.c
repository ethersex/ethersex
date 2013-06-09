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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <util/delay.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"
#include "core/eeprom.h"
#include "protocols/syslog/syslog.h"

#include "hardware/tower/tower.h"


/**
* periodic call
*/
void tower_periodic(void)
{
   if (timervalue == 0) {
      is_button_up = 0;
      is_button_down = 0;
      PIN_CLEAR(TOWER_LIFT);
      PIN_CLEAR(TOWER_LOWER);
   } else {
      timervalue--;
   }
}

/**
* initial
*/
void tower_init(void)
{
   is_button_up = 0;
   is_button_down = 0;
   PIN_CLEAR(TOWER_POWER);
}

/**
* up button (push time in ms)
*/
void push_button_up(uint16_t time)
{
   is_button_down = 0;
   is_button_up = 1;
   timervalue = time * 10;
   PIN_SET(TOWER_LIFT);
}


/**
* down button (push time in ms)
*/
void push_button_down(uint16_t time)
{
   is_button_up = 0;
   is_button_down = 1;
   timervalue = time * 10;
   PIN_SET(TOWER_LOWER);
}

/**
* set power on/off
*/
void set_power(uint8_t on)
{
   if (on == 0)
      PIN_CLEAR(TOWER_POWER);
   else
      PIN_SET(TOWER_POWER);
}


/*
  -- Ethersex META --
  header(hardware/tower/tower.h)
  mainloop(tower_periodic)
  init(tower_init)
*/



