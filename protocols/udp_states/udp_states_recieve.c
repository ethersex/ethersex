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

#include "udp_states_recieve.h"
uint8_t udp_states_register_state(uint8_t node,uint8_t type,uint8_t state_place,uint8_t scale,void (*callback)(void)){
  uint8_t i,j;
  int tmp_bs;
  udp_states_state_t tmp_state;
  tmp_state.node=node;
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
  debug_printf("UDP_STATES registert states (register_state fcn)%d\n", udp_states_n_registert_states);
  return tmp;
}

void _udp_states_init(void){
  debug_printf("UDP_STATES registert states (befor init) %x\n", udp_states_n_registert_states);
  udp_states_n_registert_states=0;
  debug_printf("UDP_STATES registert states (after init) %x\n", udp_states_n_registert_states);
  udp_states_n_registert_callbacks=0;
  uip_ipaddr_t io;
  uip_ipaddr_copy (&ip, all_ones_addr);

  uip_udp_conn_t *udp_states_connection = uip_udp_new (&ip, 0, _udp_states_process);
  
  if (!udp_states_connection) 
    return;
  
  uip_udp_bind (udp_states_connection, HTONS (UDP_STATES_PORT));
}

void _udp_states_process(void){
  if(uip_len>7&&uip_newdata()){
    uint16_t len = uip_len-5;
    uint8_t buffer[uip_len];
    memcpy(buffer, uip_appdata, uip_len);
    udp_states_packet_t packet;
    udp_states_state_t tmp_state;
    udp_states_state_t *found_state;
    packet.node = buffer[1];
    packet.type = buffer[2];
    packet.part = buffer[3];
    packet.prio_byte= buffer[4];
    packet.data=&buffer[5];
    tmp_state.node=packet.node;
    tmp_state.type=packet.type;
    tmp_state.part=packet.part;
    debug_printf("UDP_STATES %x:%x:%x %x:%x:%x\n%x,%x,%x",tmp_state.node,tmp_state.type,tmp_state.part,udp_states_states[0].node,udp_states_states[0].type,udp_states_states[0].part,buffer[6],buffer[7],buffer[8]);
    found_state=(udp_states_state_t*) bsearch((const void*)&tmp_state,(const void*)udp_states_states,udp_states_n_registert_states,sizeof(udp_states_state_t),_udp_states_compare);
    udp_states_make_state(found_state,packet,len);
    for(udp_states_state_t *i = found_state;i>=udp_states_states;i--){
      if(_udp_states_compare(i,&tmp_state)!=0)
	break;
      else{
	udp_states_make_state(i,packet,len);
      }
    }
    for(udp_states_state_t *i = found_state;i<=udp_states_states+sizeof(udp_states_state_t);i++){
      if(_udp_states_compare(i,&tmp_state)!=0)
	break;
      else{
	udp_states_make_state(i,packet,len);
      }
    }
  }
  uip_slen = 0;
}
int16_t udp_states_make_IO_value_word(uint8_t highbyte,uint8_t lowbyte)
{
  int16_t result = highbyte;
  result<<=8;
  result|=lowbyte;
  return result;
}

int16_t udp_states_make_float_value(int8_t *data, udp_states_value_t *state)
{
  int8_t diff_expo=data[2]-state->expo;
  int16_t result=data[0];
  result|=data[1];
  result<<=8;
  if(diff_expo>0){
    return (data[0]<0)?(-0x7FFF):(0x7FFF);
  }
  result = (data[0]<0)?(-result):(result);
  diff_expo=-diff_expo;
  if(diff_expo>=15)
  {
    return (data[0]<0)?(-0):(0);
  }
  result>>=diff_expo;
  return (data[0]<0)?(-result):(result);
}

void _udp_states_timer(void){
  uint8_t i;
  for(i=0;i<udp_states_n_registert_states;i++){
    if(udp_states_values[i].ttl<254)
      udp_states_values[i].ttl++;
    else if(udp_states_values[i].ttl==254){
      udp_states_values[i].prio=0;
      udp_states_values[i].ttl++;
    }
    
  }
}


uint8_t _udp_states_register_callback(void (*callback)(void)){
  uint8_t i=0;
  if(udp_states_n_registert_callbacks>=3)
    return 3;
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

inline void udp_states_make_state(udp_states_state_t *found_state,udp_states_packet_t packet,uint8_t len)
{
  udp_states_value_t *tmp_value;
  if(found_state!=NULL&&found_state->index*3<len)
    {
      debug_printf("UDP_STATES Process\n",NULL);
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
      debug_printf("TTL: %x\n",tmp_value->ttl);
      tmp_value->ttl=0;
      if(packet.type>=0xE0)
      {
	tmp_value->value=udp_states_make_IO_value_word(packet.data[found_state->index*3],packet.data[found_state->index*3+1]);
	tmp_value->expo=packet.data[found_state->index*3+2];
      }
      else
      {
	tmp_value->value=udp_states_make_float_value(packet.data[found_state->index*3],tmp_value);
      }
      if(found_state->callback<udp_states_n_registert_callbacks)
	udp_states_callbacks[found_state->callback]();
    } 
}

/*
 *  -- Ethersex META --
 *  header(protocols/udp_states/udp_states.h)
 *  init(_udp_states_init)
 *  timer(50,_udp_states_timer());
 */
