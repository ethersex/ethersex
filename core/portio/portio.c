/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#define PORTIO_CONFIG_PGM
#include "core/portio/portio.h"
#include "core/portio/named_pin.h"
#include "hardware/io_expander/hc595.h"
#include "hardware/io_expander/hc165.h"
#include "core/debug.h"

static uint8_t portio_read_port(uint8_t port);
static uint8_t portio_write_port(uint8_t port, uint8_t data);
static uint8_t portio_read_ddr(uint8_t port);
static uint8_t portio_write_ddr(uint8_t port, uint8_t data);
static uint8_t portio_read_pin(uint8_t port);


static const volatile uint8_t *ddrs[] = IO_DDR_ARRAY;
static const volatile uint8_t *ports[] = IO_PORT_ARRAY;
static const volatile uint8_t *pins[] = IO_PIN_ARRAY;

#define ACCESS_IO(x) (*(volatile uint8_t *)(x))

virtual_port_t vport[IO_PORTS];

void portio_init(void)
{
  uint8_t masks[] = IO_MASK_ARRAY;
  uint8_t i;
  for (i = 0; i < IO_HARD_PORTS; i++) {
    /* Insert the read/write handlers */
    vport[i].read_port = portio_read_port;
    vport[i].write_port = portio_write_port;
    vport[i].read_ddr = portio_read_ddr;
    vport[i].write_ddr = portio_write_ddr;
    vport[i].read_pin = portio_read_pin;
    vport[i].mask = masks[i];
  }
#ifdef HC595_SUPPORT
  for (i = IO_HARD_PORTS; i < (IO_HARD_PORTS + HC595_REGISTERS); i++) {
    memset(&vport[i], 0, sizeof(virtual_port_t));
    vport[i].write_port = hc595_write_port;
    vport[i].read_port = hc595_read_port;
  }
#endif

#ifdef HC165_SUPPORT
  for (i = IO_HARD_PORTS; i < (IO_HARD_PORTS + HC165_REGISTERS); i++) {
    vport[i].read_pin = hc165_read_pin;
  }
#endif
#   ifdef NAMED_PIN_SUPPORT
    named_pin_init();
#   endif
} 


static uint8_t 
portio_read_port(uint8_t port) 
{
  return ACCESS_IO(ports[port]);
}

static uint8_t 
portio_write_port(uint8_t port, uint8_t data) 
{
  ACCESS_IO(ports[port]) = ((uint8_t)ACCESS_IO(ports[port]) & vport[port].mask) |
                        ((uint8_t)data & ~vport[port].mask);
  return 0;
}

static uint8_t 
portio_read_ddr(uint8_t port) 
{
  return ACCESS_IO(ddrs[port]);
}

static uint8_t 
portio_write_ddr(uint8_t port, uint8_t data) 
{
  ACCESS_IO(ddrs[port]) = ((uint8_t)ACCESS_IO(ddrs[port]) & vport[port].mask) |
                       ((uint8_t)data & ~vport[port].mask);
  return 0;
}

static uint8_t 
portio_read_pin(uint8_t port) 
{
  return ACCESS_IO(pins[port]);
}
