/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyrigth (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#define RISING_EDGE(port, pin)                                               \
  (!(vpin[(port)].old_state & _BV(pin)) && (vpin[(port)].state & _BV(pin)))
#define FALLING_EDGE(port, pin)                                              \
  (!(vpin[(port)].state & _BV(pin)) && (vpin[(port)].old_state & _BV(pin)))

struct VirtualPin vpin[IO_PORTS];
const struct WatchcatReaction ecmd_react[];

#if 0
static uip_conn_t *
watchcat_do_httplog (uip_ipaddr_t *ip, client_return_text_callback_t callback, PGM_P msg)
{
  /* httplog cruft goes here ... */
  return NULL;
}
#endif

#include "user_config.h"

void watchcat_edge(uint8_t port);

void watchcat_init(void) {
  uint8_t i;
  for (i = 0; i < IO_PORTS; i++) {
    vpin[i].old_state = 0;
    vpin[i].state = 0;
    vpin[i].last_input = 0;
    vpin[i].func = NULL;
  }
  watchcat_port_init();
}

void watchcat_periodic(void) {
  uint8_t i;
  for (i = 0; i < IO_PORTS; i++) {
    /* debounce the buttons */
    if (vpin[i].last_input == vport[i].read_pin(i))
      vpin[i].state = vport[i].read_pin(i);

    vpin[i].last_input = vport[i].read_pin(i);

    /* See if something has changed since last call */
    if (vpin[i].state != vpin[i].old_state) {
      /* If there is an handler for this port, call it */
      if (vpin[i].func)
        vpin[i].func(i);
      else
        vpin[i].old_state = vpin[i].state;
    }
  }
}

void watchcat_edge(uint8_t port) {
  uint8_t i = 0;
  uint8_t tmp;

  while (1) {
    tmp = (uint8_t)pgm_read_byte(&ecmd_react[i].port);

    if (tmp == 255) {
      vpin[port].old_state = vpin[port].state;
      break;
    }

    if (tmp == port) {
      tmp = (uint8_t)pgm_read_byte(&ecmd_react[i].pin);
      uint8_t falling = pgm_read_byte(&ecmd_react[i].rising);

      if ((falling && RISING_EDGE(port, tmp)) ||
          (!falling && FALLING_EDGE(port, tmp))) {
        // debug_printf("wcat: port %d pin %d\n", port, tmp);

        uip_conn_t *(*func)(uip_ipaddr_t *, client_return_text_callback_t,
                            PGM_P, ...);
        func = (void *)pgm_read_word(&ecmd_react[i].func);

        uip_ipaddr_t ipaddr;
        memcpy_P(&ipaddr, &ecmd_react[i].address, sizeof(uip_ipaddr_t));

        const char *text =
            (const char *)pgm_read_word(&ecmd_react[i].message);

        if (func)
          func(&ipaddr, NULL, text);
      } else {
        i++;
        continue;
      }

      vpin[port].old_state &= ~_BV(tmp);
      if (vpin[port].state & _BV(tmp))
        vpin[port].old_state |= _BV(tmp);

      /* only one pin at once */
      return;
    }

    i++;
  }
}

/*
  -- Ethersex META --
  header(services/watchcat/watchcat.h)
  init(watchcat_init)
  timer(1, watchcat_periodic())
*/
