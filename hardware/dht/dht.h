/*
* Read and convert data from a DHT hygro & temp sensor
*
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef DHT_H
#define DHT_H

#include <stdint.h>

#define DHT_TYPE_11 11
#define DHT_TYPE_22 22

typedef struct
{
  uint16_t polling_delay;
  int16_t temp;
  int16_t humid;
} dht_global_t;

extern dht_global_t dht_global;

void dht_init(void);
void dht_periodic(void);

#endif /* DHT_H */
