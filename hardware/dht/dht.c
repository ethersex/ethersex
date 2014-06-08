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

#include <stdint.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "core/periodic.h"      /* for HZ */

#include "dht.h"

#ifdef DEBUG_DHT
#define DHT_DEBUG(s, args...) debug_printf("DHT " s "\n", ## args)
#else
#define DHT_DEBUG(a...)
#endif

/*
		 _________
		|  -+-+-  |
		| +-+-+-+ |
		| +-+-+-+ |
		|  -+-+-  |
		| +-+-+-+ |
		|_________|
		  | | | |
		  1 2 3 4

	1. VCC (3 to 5V power)
	2. Data out (Pullup 4,7k)
	3. Not connected
	4. Ground


On DHT-11 the 1-wire data bus is pulled up with a resistor to VCC. So
if nothing is occurred the voltage on the bus is equal to VCC.

To make the DHT-11 to send the sensor readings pull down the bus for
more than 18ms in order to give DHT time to understand it and then pull
it up for 40uS.

The response is ~54uS low and 80uS high and indicates that DHT received
the request.

The data will be packed in a packet of 5 segments of 8-bits each. Totally
5×8 =40bits. Each bit sent is a follow of ~54uS Low in the bus and ~24uS
to 70uS High depending on the value of the bit.

Bit '0' : ~54uS Low and ~24uS High
Bit '1' : ~54uS Low and ~70uS High

First two segments are Humidity read, integral & decimal. Following two
are Temperature read in Celsius, integral & decimal and the last segment
is the Check Sum which is the sum of the 4 first segments. If Check Sum's
value isn't the same as the sum of the first 4 segments that means that
data received isn't correct.

At the end of packet DHT sends a ~54uS Low level, pulls the bus to High
and goes to sleep mode.
*/

/*
Implementation based on
- Datasheet
- Code and discussions on mikrocontroller.net
- https://github.com/adafruit/DHT-sensor-library
*/

/* The packet size is 40bit but each bit consists of low and high state
so 40 x 2 = 80 transitions. Also we have 2 transistions DHT response
and 2 transitions which indicates End Of Frame. In total 84 */
#define MAXTIMINGS 84

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))


/* global variables */
#include "dht_config.h"
uint8_t dht_sensors_count = NELEMS(dht_sensors);


static void
dht_start(dht_sensor_t *sensor)
{
  DHT_DEBUG("start %S", sensor->name);

  volatile uint8_t * port = sensor->port;
  uint8_t pin  = sensor->pin;

  *(port-1) |= _BV(pin); /* OUTPUT */
  *(port-0) &= ~_BV(pin); /* LOW LEVEL */
}

static void
dht_read(dht_sensor_t *sensor)
{
  DHT_DEBUG("read %S", sensor->name);

  volatile uint8_t * port = sensor->port;
  uint8_t pin  = sensor->pin;

  *(port-0) |= _BV(pin); /* HIGH LEVEL */
  _delay_us(30);
  *(port-1) &= ~_BV(pin); /* INPUT */

  /* Read in timingss, which takes approx. 4,5 milliseconds.
   * We do not disable interrupts, because a failed read is outweighed
   * by a non-serviced interrupt. Please never enclose the for-loop
   * with an ATOMIC_BLOCK! */

  uint8_t last_state = _BV(pin);
  uint8_t j = 0;
  uint8_t data[5];
  for (uint8_t i = 0; i < MAXTIMINGS; i++)
  {
    uint8_t counter = 0;
    uint8_t current_state;
    while (last_state == (current_state = *(port-2) & _BV(pin)))
    {
      _delay_us(5);
      if (++counter == 20)
      {
        DHT_DEBUG("read timeout, edge=%u", i);
        return;                 /* timeout in conversation */
      }
    }
    last_state = current_state;

    /* ignore first three transitions */
    if ((i >= 4) && (i % 2 == 0))
    {
      /* shift each bit into the storage bytes */
      data[j / 8] <<= 1;
      if (counter > 7)          /* 7*5=35us */
        data[j / 8] |= 1;
      j++;
    }
  }

  /* check we read 40 bits and that the checksum matches */
  if ((j < 40) ||
      (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
  {
    DHT_DEBUG("read failed, bits=%u, %02X %02X %02X %02X %02X",
              j, data[0], data[1], data[2], data[3], data[4]);
    return;
  }

  int16_t t;
#if DHT_TYPE == DHT_TYPE_11
  t = data[2];
  t *= 10;
  sensor->temp = t;
  t = data[0];
  t *= 10;
  sensor->humid = t;
#elif DHT_TYPE == DHT_TYPE_22
  t = data[2] << 8 | data[3];
  if (t & 0x8000)
  {
    t &= ~0x8000;
    t = -t;
  }
  sensor->temp = t;
  t = data[0] << 8 | data[1];
  sensor->humid = t;
#endif
  DHT_DEBUG("%S t=%d, h=%d%%", sensor->name, sensor->temp, sensor->humid);
}

void
dht_init(void)
{
  uint16_t delay = DHT_POLLING_INTERVAL * HZ;

  for (uint8_t i = 0; i < NELEMS(dht_sensors); i++)
  {
    DHT_DEBUG("init %S", dht_sensors[i].name);

    volatile uint8_t * port = pgm_read_byte(&dht_sensors[i].port);
    uint8_t pin  = pgm_read_byte(&dht_sensors[i].pin);

    *(port-1) &= ~_BV(pin);
    dht_sensors[i].polling_delay = delay;
    delay += 5 * HZ;
  }
}

void
dht_periodic(void)
{
  for (uint8_t i = 0; i < NELEMS(dht_sensors); i++)
  {
    dht_sensor_t * sensor = &dht_sensors[i];

    if (sensor->polling_delay == 0)
    {
      /* read sensor data */
      dht_read(sensor);
      sensor->polling_delay = DHT_POLLING_INTERVAL * HZ;
    }
    else if (--sensor->polling_delay == 0)
    {
      dht_start(sensor);
    }
  }
}

/*
  -- Ethersex META --
  header(hardware/dht/dht.h)
  init(dht_init)
  timer(1,dht_periodic())
*/
