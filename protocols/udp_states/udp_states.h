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


#ifndef UDP_SENSOR_H
#define UDP_SENSOR_H

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/uip/uip-conf.h"
#include "config.h"
#include "core/debug.h"
#ifdef UDP_STATES_RECIEVE
#include "stdlib.h"
#endif
/*
 * Type konstants one type for unit
 */
//base units
#define IO		0xE0	//IO-subtye goes from 0xE0 to 0xFF

#define METER 			0x0	//States with Unit Meter
#define KG			0x2	//States with Unit kilo gram
#define SEC			0x4	//States with Unit second
#define	AMP			0x6	//States with Unit ampere
#define KELVIN			0x8	//States with Unit Kelvin
#define MOL			0xA	//States with Unit mol
#define	CANDELA			0xC	// States with Unit Candela
//not baseunits
#define	ONE_PER_ONE		0xE	//States with 
#define METER_PER_SECOND	0x10	//States with Unit m/s Speeds
/*
 * function Prototypes
 */
 #ifdef UDP_STATES_TRANSMIT
/*
 * rechnet daten in 24bit floating point um: VZ[1]|mantisse[15]|exponent[8]
 */
void udp_states_make_float(int8_t *data,int16_t input_data,int8_t expo);
 /*
  * transmit data
  */
  void udp_states_send(uint8_t type,uint8_t part,int8_t *data,uint8_t length);
 #endif
/*
 * to register a state to read from network, return a number to get the scaled Value
 * node: 	 from the sending node
 * type: 	 witch unit have the number
 * state_place: index of the Sensor/State in the Packet
 * scale:	value*2^(scale)
 * callback:	callback function like a interrupt
 */
 #ifdef UDP_STATES_RECIEVE
uint8_t udp_states_register_state(uint8_t node, uint8_t type,uint8_t state_place,uint8_t scale,void (*callback)(void));

/*
 *  read skaled value, input the number form the udp_states_register_state
 */

#define udp_states_get_value(index) (udp_states_values[index].value)

/*
 * read time the state updated value, input the number form the udp_states_register_state
 * 255 more then 255s old 0 <1 second old
 */

#define udp_states_get_time(index) (udp_states_values[index].ttl)

/*
 * read the exponent to scale
 */
 #define udp_states_get_expo(index) (udp_states_values[index].expo)
/*
 * internal functions
 */
void _udp_states_init(void); //initialise the udp_states machine
void _udp_states_process(void); //function wich process the recieved packets
void _udp_states_timer(void); 	//timer to make alter data
uint8_t _udp_states_register_callback(void (*callback)(void)); //function to register the calback function
int _udp_states_compare(const void *lp,const void *rp); //function to compare the states for b-search
#endif
/*
 * constants
 */
#ifndef UDP_STATES_MAX_READ_STATES
	#define UDP_STATES_MAX_READ_STATES 64
#endif
#define UDP_STATES_MAX_CALLBACKS 3

/*
 * Typedefs
 */

// tpye to descripe the packet 
 typedef struct {
	 uint8_t node; //transmitter
	 uint8_t type;	//witch Unit from State
	 uint8_t part;	//0-63 to have the capability to transmit more then 8 States
	 union {
		uint8_t prio_byte;
		struct{
			uint8_t prio_reset:1;
			uint8_t prio_not_recieved:1;
			uint8_t prio:6;
		};
	};
	 uint8_t *data;
 }udp_states_packet_t;
#ifdef UDP_STATES_RECIEVE
 //type manage states that to be recieve and processed
typedef struct {
  uint8_t node,type; //transmitter and Type
  uint8_t part:5;	//part 0 - 63
  uint8_t index:3;	//index from the Sensor
  uint8_t callback:2;	//index for the Callback function
  uint8_t value_index:6;	//index for the memory for the value
} udp_states_state_t;

//type to store data from States eg. the Value, how old the exponent and the priority
typedef struct {
  uint8_t ttl; //time to live, but counts from 0 to 255, 0 means recieved at the moment and 255 more then 255 Seconds old.
  int16_t value; //last recieved value
  int8_t expo;//exponent to the value
  uint8_t prio; //byte to store the recieved priority value
}udp_states_value_t;
//Functionen
int16_t udp_states_make_IO_value_word(uint8_t highbyte,uint8_t lowbyte); //makes a word from 2 byte NOT TESTED YET
int16_t udp_states_make_float_value(int8_t *data, udp_states_value_t *state); //store the recieved Value in the Memory
inline void udp_states_make_state(udp_states_state_t *found_state,udp_states_packet_t packet,uint8_t len); //extract the data for the State
/*
 * Glob_variabelen
 */
uint8_t udp_states_n_registert_states; //count the registert states;
uint8_t udp_states_n_registert_callbacks; //count the registert callbacks
udp_states_state_t udp_states_states[UDP_STATES_MAX_READ_STATES];s //memory for the states
udp_states_value_t udp_states_values[UDP_STATES_MAX_READ_STATES]; //memory for the values from the states
void (*udp_states_callbacks[3])(void); //memory for the callbacks
#endif
#endif
