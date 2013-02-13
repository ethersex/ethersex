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

uint8_t udp_states_make_float(uint8_t *data,int16_t input_data,int8_t expo)
 {
	 data[0]=(input_data>>8)&0xFF;
	 data[1]=(input_data)&0xFF;
	 data[2]=expo;
 }
 
 void udp_states_transmit(uint8_t type, uint8_t part, uint8_t *data, uint8_t len)
 {
	 uint8_t* packet = uip_appdata;
	 packet[0]=UDP_STATES_NODE_ID;
	 packet[1]=type;
	 packet[2]=part;
	 packet[3]=0;
	 memcpy(&packet[4],data,len);
	 uip_slen=4+len;
	 uip_udp_conn_t echo_conn;
    uip_ipaddr_copy(echo_conn.ripaddr, UDP_STATES_MULTICAST_IP_ADDR);
    echo_conn.rport = HTONS(UDP_STATES_PORT);
    echo_conn.lport = HTONS(UDP_STATES_PORT);

    uip_udp_conn = &echo_conn;
    uip_process(UIP_UDP_SEND_CONN); 
    uip_slen = 0;
}
	 
