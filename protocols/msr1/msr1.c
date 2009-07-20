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

#ifdef DEBUG_MSR1
# include "core/debug.h"
# define MSR1_DEBUG(a...)  debug_printf("msr1: " a)
#else
# define MSR1_DEBUG(a...)
#endif

#include "pinning.c"
#include "msr1.h"

#define USE_USART MSR1_USE_USART
#define BAUD MSR1_BAUDRATE
#include "core/usart.h"


/* We generate our own usart init module, for our usart port */
generate_usart_init()

uint8_t state;
struct msr1_e8_info e8_data;
struct msr1_e8_info e8_data_new;
struct msr1_c0_info c0_data;
struct msr1_c0_info c0_data_new;

void
msr1_init(void)
{
    /* Initialize the usart module */
    usart_init();

}

void
msr1_periodic(void)
{
  if (state == MSR1_REQUEST_C0) {
    state = MSR1_REQUEST_E8;
    e8_data_new.chksum = 0;
    e8_data_new.len = 0;
    usart(UDR) = 0xe8;
    MSR1_DEBUG("sent e8 command\n");
  } else {
    state = MSR1_REQUEST_C0;
    c0_data_new.chksum = 0;
    c0_data_new.len = 0;
    usart(UDR) = 0xc0;
    MSR1_DEBUG("sent c0 command\n");
  }
}


SIGNAL(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
  uint8_t data = usart(UDR);
  if (state == MSR1_REQUEST_E8) {
    e8_data_new.data[e8_data_new.len++] = data;
    e8_data_new.chksum ^= data;
    if (e8_data_new.len == 22) {
      if (e8_data_new.data[0] == 5 && e8_data_new.chksum == 0) {
        memcpy(&e8_data, &e8_data_new, sizeof(e8_data));
        e8_data.data[0] = 0;
        MSR1_DEBUG("e8_data correct\n");
      } else {
        MSR1_DEBUG("got e8 cksum: %x != %x (calc)\n", e8_data_new.data[21], e8_data_new.chksum ^ data);
        e8_data.data[0]++;
      }
    }
  } else {
    c0_data_new.data[c0_data_new.len++] = data;
    c0_data_new.chksum ^= data;
    if (c0_data_new.len == 76) {
      if (c0_data_new.data[0] == 0 && c0_data_new.chksum == 0) {
        memcpy(&c0_data, &c0_data_new, sizeof(c0_data));
        MSR1_DEBUG("c0_data correct\n");
        c0_data.data[0] = 0;
      } else {
        MSR1_DEBUG("got c0 cksum: %x != %x (calc)\n", c0_data_new.data[75], c0_data_new.chksum ^ data);
        c0_data.data[0]++;
      }
    }
  }
}

/*
  -- Ethersex META --
  header(protocols/msr1/msr1.h)
  init(msr1_init)
  timer(100, msr1_periodic())
*/
