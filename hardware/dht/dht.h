/*
* Read and convert data from a DHT hygro & temp sensors
*
* Copyright (c) 2013-14 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __DHT_H
#define __DHT_H

#include <stdint.h>

#define DHT_TYPE_11 11
#define DHT_TYPE_22 22

typedef struct
{
  volatile uint8_t* port;
  uint8_t pin;
  const char* name;
  uint16_t polling_delay;
  int16_t temp;
  int16_t humid;
} dht_sensor_t;

extern dht_sensor_t dht_sensors[];
extern uint8_t dht_sensors_count;

void dht_init(void);
void dht_periodic(void);

#endif /* __DHT_H */
