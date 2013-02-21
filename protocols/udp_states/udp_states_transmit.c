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

#include "udp_states_transmit.h"

void udp_states_make_float(int8_t *data,int16_t input_data,int8_t expo)
 {
   uint8_t i;
   int8_t s_expo=expo;
   uint16_t result=(input_data<0)?(-input_data):(input_data);
   result&=0x7FFF;
   if(result!=0){
     for(i=0;i<15&&(result&0x4000)==0;i++){
       result<<=1;
    }
   }
   else
   {
     i = 15;
   }
   result=(input_data<0)?(-result):(result);
   data[0]=(result>>8)&0xFF;	   
   data[1]=(result)&0xFF;
   data[2]=(s_expo-i);
 }
 
 void udp_states_send(uint8_t type, uint8_t part, int8_t *data, uint8_t len)
 {
	 uip_ipaddr_t mip;
	 set_UDP_STATES_MULTICAST_IP_ADDR(mip);
	 uint8_t* packet = &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
	 packet[0]=UDP_STATES_NODE_ID;
	 packet[1]=type;
	 packet[2]=part;
	 packet[3]=0;
	 memcpy(&packet[4],data,len);
	 uip_slen=4+len;
	 uip_udp_conn_t udp_states_conn;
	 uip_ipaddr_copy(udp_states_conn.ripaddr, mip );
	 udp_states_conn.rport = HTONS(UDP_STATES_PORT);
	 udp_states_conn.lport = HTONS(UDP_STATES_PORT);
	 uip_udp_conn = &udp_states_conn;
	 uip_process(UIP_UDP_SEND_CONN);
	 router_output(	);
	 uip_slen = 0;
}
	 
