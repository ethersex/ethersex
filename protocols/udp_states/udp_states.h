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
#include "config.h"
#ifdef UDP_STATES_RECIEVE
#include "stdlib.h"
#endif
/*
 * Type konstants one type for unit
 */
//base units
#define IO		0xE0 //IO-subtye goes from 0xE0 to 0xFF

#define METER 		0x1
#define KG		0x2
#define SEC		0x3
#define	AMP		0x4
#define KELVIN		0x5
#define MOL		0x6
#define	CANDELA		0x7
//not baseunits
#define	ONE_PER_ONE		0x10
#define METER_PER_SECOND	0x11
/*
 * function Prototypes
 */
 #ifdef UDP_STATES_TRANSMIT
/*
 * rechnet daten in 24bit floating point um: VZ[1]|mantisse[15]|exponent[8]
 */
 uint8_t udp_states_make_float(uint8_t *data,int16_t input_data,int8_t expo);
 /*
  * transmit data
  */
  void udp_states_send(uint8_t type,uint8_t part,uint8_t *data,uint8_t length);
 #endif
/*
 * to register a state to read from network, return a number to get the scaled Value
 * last_IP_byte: eg. 244 is the last_IP_byte from 192.168.0.244
 * type: 	 witch unit have the number
 * state_place: index of the Sensor/State in the Packet
 * scale:	value*2^(scale-64)
 * callback:	callback function like a interrupt
 */
 #ifdef UDP_STATES_RECIEVE
uint8_t udp_states_register_state(uint8_t last_IP_byte, uint8_t type,uint8_t state_place,uint8_t scale,void (*callback)(void));

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
void _udp_states_init(void);
void _udp_states_process(void);
void _udp_states_timer(void);
uint8_t _udp_states_register_callback(void (*callback)(void));
int _udp_states_compare(const void *lp,const void *rp);
int16_t _udp_states_make_value(uint8_t type,int8_t expo,uint8_t *data,uint8_t len,uint8_t index,int16_t old);
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
 typedef struct {
	 uint8_t node;
	 uint8_t type;
	 uint8_t part;
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
typedef struct {
  uint8_t node,type;
  uint8_t part:5;
  uint8_t index:3;
  uint8_t callback:2;
  uint8_t value_index:6;
} udp_states_state_t;


typedef struct {
  uint8_t ttl;
  int16_t value;
  int8_t expo;
  uint8_t prio;
}udp_states_value_t;

/*
 * Glob_variabelen
 */
uint8_t udp_states_n_registert_states;
uint8_t udp_states_n_registert_callbacks;
udp_states_state_t udp_states_states[UDP_STATES_MAX_READ_STATES];
udp_states_value_t udp_states_values[UDP_STATES_MAX_READ_STATES];
void (*udp_states_callbacks[3])(void);
#endif
#endif
