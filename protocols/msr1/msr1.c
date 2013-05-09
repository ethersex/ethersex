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
struct msr1_generic_info new_data; /* This is used for all incoming messages */
struct msr1_e8_info      msr1_e8_data;
struct msr1_generic_info msr1_c0_data;
struct msr1_generic_info msr1_48_data;
struct msr1_generic_info msr1_50_data;

void
msr1_init(void)
{
    /* Initialize the usart module */
    usart_init();

}

void
msr1_periodic(void)
{
  new_data.chksum = 0;
  new_data.len = 0;

  if (state == MSR1_REQUEST_C0) {
    state = MSR1_REQUEST_E8;
    usart(UDR) = 0xe8;
    MSR1_DEBUG("sent e8 command\n");
  } else if (state == MSR1_REQUEST_E8) {
    state = MSR1_REQUEST_48;
    usart(UDR) = 0x48;
    MSR1_DEBUG("sent 48 command\n");
  } else if (state == MSR1_REQUEST_48) {
    state = MSR1_REQUEST_50;
    usart(UDR) = 0x50;
    MSR1_DEBUG("sent 50 command\n");
  } else {
    state = MSR1_REQUEST_C0;
    usart(UDR) = 0xc0;
    MSR1_DEBUG("sent c0 command\n");
  }
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
  new_data.data[new_data.len++] = data;
  new_data.chksum ^= data;

  if (state == MSR1_REQUEST_E8) {
    if (new_data.len == 22) {
      if (new_data.data[0] == 5 && new_data.chksum == 0) {
        memcpy(&msr1_e8_data, &new_data, sizeof(msr1_e8_data));
        msr1_e8_data.data[0] = 0;
        MSR1_DEBUG("msr1_e8_data correct\n");
      } else {
        MSR1_DEBUG("got e8 cksum: %x != %x (calc)\n", new_data.data[21], 
                   new_data.chksum ^ data);
        msr1_e8_data.data[0]++;
      }
    }
  } else if (state == MSR1_REQUEST_48) {
    if (new_data.len == 76) {
        memcpy(&msr1_48_data, &new_data, sizeof(msr1_48_data));
        MSR1_DEBUG("48_data ready\n");
        msr1_48_data.data[0] = 0;
        /* FIXME: here we have to determine if there is a checksum */
    }
  } else if (state == MSR1_REQUEST_50) {
    if (new_data.len == 76) {
        memcpy(&msr1_50_data, &new_data, sizeof(msr1_48_data));
        MSR1_DEBUG("50_data ready\n");
        msr1_50_data.data[0] = 0;
        /* FIXME: here we have to determine if there is a checksum */
    }
  } else {
    if (new_data.len == 76) {
      if (new_data.data[0] == 0 && new_data.chksum == 0) {
        memcpy(&msr1_c0_data, &new_data, sizeof(msr1_c0_data));
        MSR1_DEBUG("msr1_c0_data correct\n");
        msr1_c0_data.data[0] = 0;
      } else {
        MSR1_DEBUG("got c0 cksum: %x != %x (calc)\n", new_data.data[75], new_data.chksum ^ data);
        msr1_c0_data.data[0]++;
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
