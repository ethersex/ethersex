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

#include "udp_states.h"
uint8_t udp_states_register_state(uint8_t last_IP_byte,\
				uint8_t type,\
				uint8_t state_place,\
				uint8_t scale,void (*callback)(void)){
	uint8_t i,j;
	int tmp_bs;
	udp_states_state_t tmp_state;
	tmp_state.node=last_IP_byte;
	tmp_state.type=type;
	tmp_state.part=state_place/8;
	tmp_state.index=state_place%8;
	for(i = 0;i<udp_states_n_registert_states;i++){
		tmp_bs=_udp_states_compare(&udp_states_states[i],&tmp_state);
		if(tmp_bs<0)
			break;
		else if(tmp_bs ==0){
			if(tmp_state.index==udp_states_states[i].index)
				return i;
			else if(tmp_state.index>udp_states_states[i].index)
				break;
		}
	}
	for(j=udp_states_n_registert_states;j>i;j--){
		udp_states_states[j]=udp_states_states[j-1];
	}
	udp_states_states[j]=tmp_state;
	uint8_t tmp= _udp_states_register_callback(callback);
	if(tmp>UDP_STATES_MAX_CALLBACKS)
		return 255;
	udp_states_states[j].value_index=udp_states_n_registert_states;
	udp_states_values[udp_states_n_registert_states].ttl=255;
	udp_states_values[udp_states_n_registert_states].expo=scale;
	udp_states_values[udp_states_n_registert_states].prio=0;
	udp_states_states[j].callback=tmp;
	tmp=udp_states_n_registert_states++;
	return tmp;
}

void _udp_states_init(void){
  udp_states_n_registert_states=0;
  udp_states_n_registert_callbacks=0;
  uip_ipaddr_t ip;
  uip_ipaddr_copy (&ip, all_ones_addr);

  uip_udp_conn_t *udp_states_connection = uip_udp_new (&ip, 0, _udp_states_process);

  if (!udp_states_connection) 
	return;

  uip_udp_bind (udp_states_connection, HTONS (UDP_STATES_PORT));
}

void _udp_states_process(void){
	uint16_t len = uip_len-4;
	uint8_t buffer[uip_len];
	memcpy(buffer, uip_appdata, uip_len);
	udp_states_packet_t packet;
	udp_states_state_t tmp_state;
	udp_states_state_t *found_state;
	udp_states_value_t *tmp_value;
	packet.node = buffer[0];
	packet.type = buffer[1];
	packet.part = buffer[2];
	packet.prio_byte= buffer[3];
	packet.data=&buffer[4];
	tmp_state.node=packet.node;
	tmp_state.type=packet.type;
	tmp_state.part=packet.part;
	found_state=(udp_states_state_t*) bsearch(
			(const void*)&tmp_state,\
			(const void*)udp_states_states,\
			udp_states_n_registert_states,\
			sizeof(udp_states_state_t),\
			_udp_states_compare);
	if(found_state!=NULL&&found_state->index*3<len)
	{
		tmp_value=&udp_states_values[found_state->value_index];
		if(packet.prio>0)
		{
			if(packet.prio_reset)
			{
				tmp_value->prio=0;
			}
			else
			{
				tmp_value->prio=packet.prio;
			}
			if(packet.prio_not_recieved>0)
			{
				return;
			}
		}
		else if(tmp_value->prio>0)
		{
			return;
		}
		tmp_value->ttl=0;
		tmp_value->value=_udp_states_make_value(tmp_state.type,\
								tmp_value->expo,packet.data,\
								len,found_state->index,tmp_value->value);
		if(tmp_state.callback>0)
			udp_states_callbacks[tmp_state.callback]();
		
	}
}

int16_t _udp_states_make_value(uint8_t type,int8_t expo,uint8_t *data,uint8_t len,uint8_t index,int16_t old)
{
	int16_t result;
	int8_t tmp_expo;
	index=index*3;
	if((type&0xE0)>0)
	{
		result = 0;
		if(expo>23) //geht nicht da immer 3 bytes zusammen addresiert werden und hier expo alls verzug herhält
			return old; //gibt den alten wert zurück
		else
		{
			for(uint8_t i=0;i<(expo%8==0)?(2):(3);i++)
			{
				result|=(data[index+i]>>expo%8)<<(i*8); //rückt zuerst zurecht, dann auf passendes byte
			}
		}
		return result;
	}
	else
	{
		result=(data[0]<<8)|(data[1]);
		tmp_expo=data[index+2];
		int8_t diff_expo=tmp_expo-expo;
		if(diff_expo<0)
		{
			if(result<0)
				return -((-result)>>(-diff_expo));
			else
				return result>>(-diff_expo);
		}
		else
		{
			if(result >= 0)
			{
				if(result>0x7FFF>>diff_expo)
				{
					return 0x7FFF;
				}
				else
				{
					return result>>diff_expo;
				}
			}
			else
			{
				if(-result>0x7FFF>>diff_expo)
				{
					return -0x7FFF;
				}
				else
				{
					return (-(-result)>>diff_expo);
				}
			}
		}
	}
}


void _udp_states_timer(void){
	uint8_t i;
	for(i=0;i<udp_states_n_registert_states;i++){
		if(udp_states_values[i].ttl<254)
			udp_states_values[i].ttl++;
		else{
			udp_states_values[i].prio=0;
			udp_states_values[i].ttl++;
		}
			
	}
}


uint8_t _udp_states_register_callback(void (*callback)(void)){
	uint8_t i=0;
	if(udp_states_n_registert_callbacks>=3)
		return 255;
	if(callback==NULL)
		return 0;
	for(i = 0; i<udp_states_n_registert_callbacks;i++){
		if(udp_states_callbacks[i]==callback)
			return i;
	}
	udp_states_callbacks[i]=callback;
	udp_states_n_registert_callbacks++;
	return i;
}

int _udp_states_compare(const void *lp,const void *rp){
	udp_states_state_t *left=(udp_states_state_t *)lp;
	udp_states_state_t *right=(udp_states_state_t *)rp;
	if(left->node>right->node){
		return 1;
	}
	else if(left->node<right->node){
		return -1;
	}
	else{
		if(left->type>right->type)
			return 1;
		else if(left->type<right->type)
			return -1;
		else{
			if(left->part>right->part)
				return 1;
			else if(left->part<right->part)
				return -1;
			else{
				return 0;
			}
		}
	}
}

/*
  -- Ethersex META --
  init(_udp_states_init)
  timer(50,_udp_states_timer());
*/
