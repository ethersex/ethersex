/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include "core/bit-macros.h"
#include "core/util/byte2hex.h"

static uint8_t print_port(char *output, uint8_t len, uint8_t port, uint8_t value) 
{
#ifndef TEENSY_SUPPORT
        return snprintf_P(output, len,
                PSTR("port %d: 0x%02x"),
                port, value);
#else
        memcpy_P(output, PSTR("port P: 0x"), strlen("port P: 0x"));
        /* Convert to number :) */
        output[5] = port + 48;
        byte2hex(value, output + 10);
        return 12;
#endif
}


