/*
*
* Copyright (c) 2012 by Marian Kerler myasuro@gmail.com
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

#include "config.h"
#include "udp_states.h"
uint8_t index;
void udp_states_example_init(void){
  DDRD=0xFF;
  index=udp_states_register_state(1,0xe0,0,0,NULL);
  debug_printf("UDP_STATES_EXAMPLE_Index %d", index);
}

void udp_states_sample(void)
{
		static int16_t i = 0;
		int8_t data[3];
		udp_states_value_t state;
		state.expo=-10;
		udp_states_make_float(data,i,0);
		udp_states_send(0x2F,0,data,3);
		i+=1;
}

void udp_states_example_recieve(void)
{
  if(udp_states_get_time(index)<10)
    PORTD=~((uint8_t)(udp_states_get_value(index)&0xFF)|0x01);
  else
    PORTD=~0;
}

/*
  -- Ethersex META --
  startup(udp_states_example_init)
  timer(50,udp_states_sample());
  timer(50,udp_states_example_recieve());
*/
