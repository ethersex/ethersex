/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include "portio.h"
#include "debug.h"

static const volatile uint8_t *ddrs[] = IO_DDR_ARRAY;
static const volatile uint8_t *ports[] = IO_PORT_ARRAY;

#define ACCESS_IO(x) (*(volatile uint8_t *)(x))

/* update port information (PORT and DDR) from global status */
void portio_update(void)
/* {{{ */ {

    for (uint8_t i = 0; i < IO_PORTS; i++) {

#       ifdef DEBUG_PORTIO
        if (ACCESS_IO(ddrs[i]) != cfg.options.io_ddr[i])
            debug_printf("io: ddr %d changed to %02x\n", i, cfg.options.io_ddr[i]);
        if (ACCESS_IO(ports[i]) != cfg.options.io[i])
            debug_printf("io: port %d changed to %02x\n", i, cfg.options.io[i]);
#       endif

        ACCESS_IO(ddrs[i]) = cfg.options.io_ddr[i];
        ACCESS_IO(ports[i]) = cfg.options.io[i];
    }

} /* }}} */
