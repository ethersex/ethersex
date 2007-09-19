/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../portio.h"
#include "../net/ecmd_sender_net.h"
#include "watchcat.h"

#ifdef WATCHCAT_SUPPORT

#define RISING_EDGE(port, pin) (!(vpin[(port)].old_state & _BV(pin)) \
                                && (vpin[(port)].state & _BV(pin)))
#define FALLING_EDGE(port, pin) (!(vpin[(port)].state & _BV(pin)) \
                                && (vpin[(port)].old_state & _BV(pin)))

static struct VirtualPin vpin[IO_PORTS];

void watchcat_edge(uint8_t pin);

void 
watchcat_init(void)
{
  // FIXME: HACK
  DDRA = 0x55;
  PORTA = 0xAA;
  uint8_t i;
  for(i = 0; i < IO_PORTS; i++) {
    vpin[i].old_state = 0;
    vpin[i].state = 0;
    vpin[i].last_input = 0;
    vpin[i].func = NULL;
  }
  vpin[0].func = watchcat_edge;
}

void
watchcat_periodic(void)
{
  uint8_t i;
  for (i = 0; i < IO_PORTS; i++) {
    /* debounce the buttons */
    if ( vpin[i].last_input == portio_input(i)) 
      vpin[i].state = portio_input(i);
    
    vpin[i].last_input = portio_input(i);
    
    /* See if something has changed since last call */
    if ( vpin[i].state != vpin[i].old_state ) {
      /* If there is an handler for this port, call it */
      if ( vpin[i].func )   
        vpin[i].func(i);
      else
        vpin[i].old_state = vpin[i].state;
    }
  }
}

void 
watchcat_edge(uint8_t pin) 
{
  if ( FALLING_EDGE(pin, 1) ) {
    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, 192,168,100,2);
    ecmd_sender_send_command(&ipaddr, PSTR("PORTA CHANGED\n"));
  }
  vpin[pin].old_state = vpin[pin].state;
}

#endif

