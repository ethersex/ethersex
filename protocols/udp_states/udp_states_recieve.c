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
// register state to recieve
/*node: from witch node the state is transmittet
 * type: wich type the state has
 * state_place: the place in the packet from the state
 * scale: witch is the preferd exponent from the state
 * callback: the callback function , wich is executet the state is recieved
 */

uint8_t udp_states_register_state(uint8_t node,uint8_t type,uint8_t state_place,uint8_t scale,void (*callback)(void)){
  uint8_t i,j;
  int tmp_bs;
  uint8_t tmp= _udp_states_register_callback(callback); //register the callback function
  if(udp_states_n_registert_states>=(UDP_STATES_MAX_READ_STATES-1)||tmp>=UDP_STATES_MAX_CALLBACKS) //test if the operation is able to do
    return 255; //returns a bad index
  udp_states_state_t tmp_state; //make state to compare to the other
  tmp_state.node=node;	//write node,type, calculate the part and index
  tmp_state.type=type;
  tmp_state.part=state_place/8;
  tmp_state.index=state_place%8;
  for(i = 0;i<udp_states_n_registert_states;i++){ //search the place who the new state have to be
    tmp_bs=_udp_states_compare(&udp_states_states[i],&tmp_state); //compare the values
    if(tmp_bs<0) //if the new state bigger then the other, then break;
      break;
    else if(tmp_bs ==0){ //if the the same, so only the index of the state is a other
      if(tmp_state.index==udp_states_states[i].index) //test the indexies
	return udp_states_states[i].index;	//if the same, dont do anything and return the place of it.
      else if(tmp_state.index>udp_states_states[i].index)
	break;
    }
  }
  for(j=udp_states_n_registert_states;j>i;j--){ //if the state between the other states, copy the bigger states to backward
    udp_states_states[j]=udp_states_states[j-1];
  } //if completet the right place are free
  udp_states_states[j]=tmp_state; //write the data to the free place
  udp_states_states[j].value_index=udp_states_n_registert_states; //write the value index
  udp_states_values[udp_states_n_registert_states].ttl=255; //write ttl to no good data
  udp_states_values[udp_states_n_registert_states].expo=scale; //write the preferd scale
  udp_states_values[udp_states_n_registert_states].prio=0; //write no priority was recieved
  udp_states_states[j].callback=tmp; //write index off the callback fcn.
  tmp=udp_states_n_registert_states++;
  debug_printf("UDP_STATES registert states (register_state fcn)%d\n", udp_states_n_registert_states);
  return tmp; //return the index of the value
}

void _udp_states_init(void){
  debug_printf("UDP_STATES registert states (befor init) %x\n", udp_states_n_registert_states);
  udp_states_n_registert_states=0; //no states are registert
  debug_printf("UDP_STATES registert states (after init) %x\n", udp_states_n_registert_states);
  udp_states_n_registert_callbacks=0; //no callbacks registert
  uip_ipaddr_t io;
  uip_ipaddr_copy (&ip, all_ones_addr);

  uip_udp_conn_t *udp_states_connection = uip_udp_new (&ip, 0, _udp_states_process); //register the protocol
  
  if (!udp_states_connection) 
    return;
  
  uip_udp_bind (udp_states_connection, HTONS (UDP_STATES_PORT)); //bind the protocol to the Port
}

void _udp_states_process(void){
  if(uip_len>7&&uip_newdata()){ // a minimum packet of udp_states are 8 byte -> smaller are a corrupted packet
    uint16_t len = uip_len-5; //header length are 5 byte so the states are uip_len - 5
    uint8_t buffer[uip_len]; //get memory for the packet
    memcpy(buffer, uip_appdata, uip_len); //get the packet
    udp_states_packet_t packet; // place for the header
    udp_states_state_t tmp_state; // a state to compare
    udp_states_state_t *found_state; //pointer to the found state
    packet.node = buffer[1]; //make the packet
    packet.type = buffer[2];
    packet.part = buffer[3];
    packet.prio_byte= buffer[4];
    packet.data=&buffer[5];
    tmp_state.node=packet.node; //make the compare state
    tmp_state.type=packet.type;
    tmp_state.part=packet.part;
    debug_printf("UDP_STATES %x:%x:%x %x:%x:%x\n%x,%x,%x",tmp_state.node,tmp_state.type,tmp_state.part,udp_states_states[0].node,udp_states_states[0].type,udp_states_states[0].part,buffer[6],buffer[7],buffer[8]);
    found_state=(udp_states_state_t*) bsearch((const void*)&tmp_state,(const void*)udp_states_states,udp_states_n_registert_states,sizeof(udp_states_state_t),_udp_states_compare);
    udp_states_make_state(found_state,packet,len); //make state from the packet
    for(udp_states_state_t *i = found_state;i>=udp_states_states;i--){ //search other states in the packet
      if(_udp_states_compare(i,&tmp_state)!=0) //if compare from the state is not null for the state is a other packet needed
	break;
      else{
	udp_states_make_state(i,packet,len); //so make the state
      }
    }
    for(udp_states_state_t *i = found_state;i<=udp_states_states+sizeof(udp_states_state_t);i++){ //search in the other direction
      if(_udp_states_compare(i,&tmp_state)!=0) //like the other code
	break;
      else{
	udp_states_make_state(i,packet,len); // like the other code
      }
    }
  }
  uip_slen = 0; //all data procecced
}

int16_t udp_states_make_IO_value_word(uint8_t highbyte,uint8_t lowbyte) //two bytes to one word
{
  int16_t result = highbyte; //write first highbyte
  result<<=8; //shift it to the right place
  result|=lowbyte;// write in the low byte the lowbyte
  return result; //return value
}
/*
 * float in udp_states means one sign bit + 15 value bits with MSB, (diffrent to the ISO) + signed byte exponent
 */
int16_t udp_states_make_float_value(int8_t *data, udp_states_value_t *state)
{
  int8_t diff_expo=data[2]-state->expo;//write expo
  int16_t result=data[0]; //write first bytes
  result<<=8; //shift it
  result|=data[1];//write second byte
  if(diff_expo>0){ //if the choosen expo in the state smaller the recieved exponent we woud have a overflow so return the max bzw. the min .
    return (data[0]<0)?(-0x7FFF):(0x7FFF);
  }
  result = (data[0]<0)?(-result):(result); //we have to extract the sign
  diff_expo=-diff_expo; //then we use the abs to shift the bits to the right position
  if(diff_expo>=15) //controll if we had a underflow (msb < lsb)
  {
    return (data[0]<0)?(-0):(0); //return a signed zero
  }
  result>>=diff_expo; //shift the result to the right place
  return (data[0]<0)?(-result):(result);//return the value WITH sign
}
/*
 * function for the timer, also if the dates are valid + reset the priority bits
 */
void _udp_states_timer(void){
  uint8_t i;
  for(i=0;i<udp_states_n_registert_states;i++){ //crawl to all registert states
    if(udp_states_values[i].ttl<254) //if the ttl smaller then 254
      udp_states_values[i].ttl++; //then increment the ttl
    else if(udp_states_values[i].ttl==254){ //else if the ttl is 254
      udp_states_values[i].prio=0; //reset the prio byte
      udp_states_values[i].ttl++; //and increment the ttl
    }
    
  }
}

/*
 * fcn to register callback function
 */
uint8_t _udp_states_register_callback(void (*callback)(void)){
  uint8_t i=0;
  for(i = 0; i<udp_states_n_registert_callbacks;i++){ //search if the same callback is inserted
    if(udp_states_callbacks[i]==callback)
      return i; //if yes return the index
  }
  if(udp_states_n_registert_callbacks>=UDP_STATES_MAX_CALLBACKS) //if none index with the same callback-pointer test of free space for the callback is there if none then return failure value
    return UDP_STATES_MAX_CALLBACKS;
  udp_states_callbacks[i]=callback; //if free space, then write the callback-pointer to the space
  udp_states_n_registert_callbacks++;
  return i;//return the index of the callback fcn.
}
/*
 * internal fcn to use bsearch
 * its compares the node id, type id,and part id, NOT index because of bsearch
 */
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
 * fcn uses data, to generate the data from the state
 */
inline void udp_states_make_state(udp_states_state_t *found_state,udp_states_packet_t packet,uint8_t len)
{
  udp_states_value_t *tmp_value;
  if(found_state!=NULL&&found_state->index*3<len)
    {
      debug_printf("UDP_STATES Process\n",NULL);
      tmp_value=&udp_states_values[found_state->value_index];
      if(packet.prio>0) //if packet with prio flag then
      {
	if(packet.prio_reset) //if the reset flag set reset the prio flag in the state
	{
	  tmp_value->prio=0;
	}
	else
	{
	  tmp_value->prio=packet.prio;//set the prio flag of the state
	}
	if(packet.prio_not_recieved>0) //if not recieved flag is set, dont reset ttl and value
	{
	  return;
	}
      }
      else if(tmp_value->prio>0) //if state is with prio then only use packet with prio flag
      {
	return;
      }
      debug_printf("TTL: %x\n",tmp_value->ttl);
      tmp_value->ttl=0; //reset ttl
      if(packet.type>=0xE0) //if type is in IO-subset then use make IO_VALUE
      {
	tmp_value->value=udp_states_make_IO_value_word(packet.data[found_state->index*3],packet.data[found_state->index*3+1]);
	tmp_value->expo=packet.data[found_state->index*3+2];
      }
      else 
      {
	tmp_value->value=udp_states_make_float_value(packet.data[found_state->index*3],tmp_value); //make float value
      }
      if(found_state->callback<udp_states_n_registert_callbacks) //if the callback index is good then run the calback fcn. <
	udp_states_callbacks[found_state->callback]();
    } 
}

/*
 *  -- Ethersex META --
 *  header(protocols/udp_states/udp_states.h)
 *  init(_udp_states_init)
 *  timer(50,_udp_states_timer());
 */
