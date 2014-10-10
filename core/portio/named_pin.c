/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2014 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef NELEMS
#define NELEMS(x) (sizeof(x)/sizeof(x[0]))
#endif


void
named_pin_init(void)
{
  /* Parse pin table */
  for (uint8_t i = 0; i < NELEMS(portio_pincfg); i++)
  {
    uint8_t port = pgm_read_byte(&portio_pincfg[i].port);
    uint8_t pin = pgm_read_byte(&portio_pincfg[i].pin);
    uint8_t input = pgm_read_byte(&portio_pincfg[i].input);
    uint8_t active_high = pgm_read_byte(&portio_pincfg[i].active_high);

    /* Set Input/Output */
    if (input)
      vport[port].write_ddr(port, vport[port].read_ddr(port) & ~_BV(pin));
    else
    {
      if (active_high == 0)
        vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));
      vport[port].write_ddr(port, vport[port].read_ddr(port) | _BV(pin));
    }

    /* If input and active low set pullup */
    if (input && (active_high == 0))
      vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));
  }
}


uint8_t
named_pin_by_name(const char *name)
{
  for (uint8_t i = 0; i < NELEMS(portio_pincfg); i++)
  {
    const char *tmp = (const char *) pgm_read_word(&portio_pincfg[i].name);
    if (strcmp_P(name, tmp) == 0)
      return i;
  }
  return 255;
}


uint8_t
named_pin_by_pin(uint8_t port, uint8_t pin)
{
  for (uint8_t i = 0; i < NELEMS(portio_pincfg); i++)
  {
    uint8_t t_port = pgm_read_byte(&portio_pincfg[i].port);
    uint8_t t_pin = pgm_read_byte(&portio_pincfg[i].pin);
    if (pin == t_pin && port == t_port)
      return i;
  }

  return 255;
}

#endif /* PORTIO_SUPPORT */
