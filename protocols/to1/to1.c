/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"

#ifdef DEBUG_TO1
# include "core/debug.h"
# define TO1_DEBUG(a...)  debug_printf("to1: " a)
#else
# define TO1_DEBUG(a...)
#endif

#include "pinning.c"
#include "to1.h"

#define USE_USART TO1_USE_USART
#define BAUD TO1_BAUDRATE
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

struct to1_info to1_new;
struct to1_data to1_sensors[TO1_SENSOR_COUNT];


void
to1_init(void)
{
    /* Initialize the usart module */
    usart_init();
    to1_new.len = 255;
    uint8_t i;
    for (i = 0; i < TO1_SENSOR_COUNT; i++)
      to1_sensors[i].timeout = 0;
}

void
to1_periodic(void)
{
    uint8_t i;
    for (i = 0; i < TO1_SENSOR_COUNT; i++)
      if (to1_sensors[i].timeout) to1_sensors[i].timeout--;
}

ISR(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
  uint8_t data = usart(UDR);

  if (to1_new.len == 255) {
    if (to1_new.chksum == 0xff && data == 0x02) { /* A new packet has started */
      to1_new.len = 0;
      to1_new.chksum = 0;
    } else 
      to1_new.chksum = data; /* abuse chksum field */
    TO1_DEBUG("got %x\n", data);
    return;
  } else { /* Packet has started */
    to1_new.data[to1_new.len++] = data;
    if (to1_new.len == 17) {
      if (data == to1_new.chksum) {
        uint16_t serial = to1_new.data[2] << 8 | to1_new.data[3];
        TO1_DEBUG("chksum is correct (serial: %d)\n", serial);
        /* No we save the data */
        uint8_t i, save_sensor = 0xff;
        /* Search if the sensor has already an index */
        for (i = 0; i < TO1_SENSOR_COUNT; i++) {
          uint16_t old_serial = to1_sensors[i].data[1] << 8 | to1_sensors[i].data[2];
          if (old_serial == serial) {
            save_sensor = i;
            break;
          }
        }
        if (save_sensor == 0xff) {
          TO1_DEBUG("new sensor detected\n");
          /* Search for empty slot */
          for (i = 0; i < TO1_SENSOR_COUNT; i++) {
            if (to1_sensors[i].timeout == 0)  {
              save_sensor = i;
              break;
            }
          }
          if (save_sensor == 0xff) {
            TO1_DEBUG("No slot available\n");
            return;
          }
        }
        TO1_DEBUG("saved in slot %d\n", save_sensor);
        to1_sensors[save_sensor].timeout = 0xff;
        memcpy(to1_sensors[save_sensor].data, &to1_new.data[1], 15);
      } else
        TO1_DEBUG("chksum is incorrect %x != %x\n", data, to1_new.chksum);
      to1_new.len = 255;
    } else {
      to1_new.chksum += data;
    }
  }
}

/*
  -- Ethersex META --
  header(protocols/to1/to1.h)
  init(to1_init)
  timer(50, to1_periodic())
*/
