/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
#include "syslog/syslog.h"

/* define macros, if debug is enabled */
#ifdef DEBUG
    #define debug_printf(s, args...) printf_P(PSTR("D: " s), ## args)

#ifdef DEBUG_USE_SYSLOG
    #define debug_init() syslog_debug_init()
    #define debug_process() do { } while(0)
#else  /* not DEBUG_USE_SYSLOG */
    #define debug_init() debug_init_uart()
    #define debug_process() debug_process_uart()
#endif	/* not DEBUG_USE_SYSLOG */

#else  /* not DEBUG */
    #define debug_printf(...) do { } while(0)
    #define debug_init(...) do { } while(0)
    #define debug_process(...) do { } while(0)
#endif /* not DEBUG */

/* Use 115200 baud on 20 MHz Ethersex,
   38400 baud on slower devices like ATmega8. */
#if F_CPU < 20000000
# define DEBUG_BAUDRATE 38400
#else
# define DEBUG_BAUDRATE 115200
#endif

/* prototypes */
void debug_init_uart(void);
void debug_process_uart(void);

#endif /* _DEBUG_H */
