/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#ifdef PORTIO_SUPPORT

#include "named_pin.h"
#include "portio.h"
#include "user_config.c"

void
named_pin_init(void)
{
  /* Parse pin table */
  uint8_t i = 0, port, pin, input;
  while (1) {
    port = pgm_read_byte(&portio_pincfg[i].port);
    if (port == 255)
      break;
    pin = pgm_read_byte(&portio_pincfg[i].pin);

    /* Set Input/Output */
    input = pgm_read_byte(&portio_pincfg[i].input); 
    if (input)
      vport[port].write_ddr(port, vport[port].read_ddr(port) & ~_BV(pin));
    else
      vport[port].write_ddr(port, vport[port].read_ddr(port) | _BV(pin));

    /* If input and active low set pullup */
    if (input && (pgm_read_byte(&portio_pincfg[i].active_high) == 0))
      vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));
    i++;
  }
 } 
/* */

uint8_t
named_pin_by_name(const char *name) 
{
  uint8_t i = 0;
  const char *tmp;
  while (1) {
    tmp = (const char *)pgm_read_word(&portio_pincfg[i].name);
    if (tmp == NULL)
      break;
    if (strcmp_P(name, tmp) == 0)
      return i;
    i++;
  }
  return 255;
}
/* */

uint8_t
named_pin_by_pin(uint8_t port, uint8_t pin)
{
  uint8_t i = 0;
  uint8_t t_port, t_pin;
  while (1) {
    t_port = pgm_read_byte(&portio_pincfg[i].port);
    t_pin = pgm_read_byte(&portio_pincfg[i].pin);
    if (t_port == 255)
      return 255;
    if (pin == t_pin && port == t_port)
      return i;
    i++;
  }
  return 255;
}
/* */

#endif /* PORTIO_SUPPORT */

