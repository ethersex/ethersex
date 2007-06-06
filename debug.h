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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#include <avr/pgmspace.h>
#include "bit-macros.h"
#include "global.h"

/* define macros, if debug is enabled */
#ifdef DEBUG
    #define debug_printf(s, args...) printf_P(PSTR("D: " s), ## args)
    #define debug_init() DEBUG_INIT_UART()
    #define debug_process() DEBUG_PROCESS_UART()
#else
    #define debug_print(s)
    #define debug_printf(...)
    #define debug_init(...)
    #define debug_process(...)
#endif /* DEBUG */

/* use 115200 baud at 20mhz (see datasheet for other values) */
#define DEBUG_UART_UBRR 10

/* prototypes */
void DEBUG_INIT_UART(void);
void DEBUG_PROCESS_UART(void);

#endif /* _DEBUG_H */
