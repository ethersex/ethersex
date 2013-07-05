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

void 
udp_states_make_float(int8_t *data,int16_t input_data,int8_t expo)
 {
   uint8_t i;
   int8_t s_expo=expo; //exponent from the input data
   uint16_t result=(input_data<0)?(-input_data):(input_data); //extract VZ
   result&=0x7FFF;
   if(result!=0){ //don*t need to shift for a 0 (becouse i woud shift 15 Times for a Zero)
     for(i=0;i<15&&(result&0x4000)==0;i++){//shift the most signifikant one to the second Bit from the Word
       result<<=1;
    }
   }
   else
   {
     s_expo=-0x7F; //if input Data zero the expronent ist smallest possible
     i=0;
   }
   result=(input_data<0)?(-result):(result);
   data[0]=(result>>8)&0xFF;	   
   data[1]=(result)&0xFF;
   data[2]=(s_expo-i);
 }
 
 void 
 udp_states_send(uint8_t type, uint8_t part, int8_t *data, uint8_t len)
 {
	 uip_ipaddr_t mip;
	 set_UDP_STATES_MULTICAST_IP_ADDR(mip); //Multicast Address from the make menuconfig
	 uint8_t* packet = &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN+ len]; //make memory for Packet
	 packet[0]=UDP_STATES_NODE_ID; //Write own NODE ID
	 packet[1]=type;		//Write Type
	 packet[2]=part;		//Write Part
	 packet[3]=0;			//No priority because is only to debug and testing
	 memcpy(&packet[4],data,len);	//copy all data to the packet memory
	 uip_slen=4+len;		//write the length 
	 uip_udp_conn_t udp_states_conn;	//get space for the connection
	 uip_ipaddr_copy(udp_states_conn.ripaddr, mip ); //set reciever IP
	 udp_states_conn.rport = HTONS(UDP_STATES_PORT); //set recieve ports
	 udp_states_conn.lport = HTONS(UDP_STATES_PORT); //set send port
	 uip_udp_conn = &udp_states_conn;	// write pointer to value to the Stack
	 uip_process(UIP_UDP_SEND_CONN);	//process the Packet and Send
	 router_output(	);			// no it goes to the nic
	 uip_slen = 0;				//set the packet length to send to zero
}
	 
