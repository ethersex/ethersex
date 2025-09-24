/*
 *
 * Copyrigth (c) 2019 by Moritz Wenk <max-1973@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <avr/pgmspace.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "core/portio/portio.h"
#include "protocols/uip/uip.h"
#include "watchcat.h"
#include "watchcat_init.h"

extern struct VirtualPin vpin[IO_PORTS];
extern const struct WatchcatReaction ecmd_react[] PROGMEM;

/*
 * initialize used pins for input (active low)
 * called in portio_init!
 *
 */

void watchcat_portio_init(void) {

  // debug_printf("wcat: pio\n");

  uint8_t port = 0;
  uint8_t pin = 0;
  uint8_t i = 0;

  while (1) {
    port = (uint8_t)pgm_read_byte(&ecmd_react[i].port);

    if (port == 255) {
      break;
    }

    pin = (uint8_t)pgm_read_byte(&ecmd_react[i].pin);

    // debug_printf("wcat: pio %d %d %d\n", i, port, pin);

    // set pullup (pin is active low)
    vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));

    // set pin for input
    vport[port].write_ddr(port, vport[port].read_ddr(port) & ~_BV(pin));

    // pin in die mask der internen Pins aufnehmen
    // vport[port].mask |= _BV(pin);

    vpin[port].old_state = vport[port].read_pin(port);
    vpin[port].last_input = vpin[port].old_state;
    vpin[port].state = vpin[port].old_state;

    i++;
  }

  // debug_printf("wcat: pio done\n");
}
