/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#include <avr/pgmspace.h>
#include "core/global.h"
#include "protocols/syslog/syslog_debug.h"

#define noinline __attribute__((noinline))

/* define macros, if debug is enabled */
#ifdef DEBUG
#ifndef TEENSY_SUPPORT
#define debug_printf(s, args...) printf_P(PSTR("D: " s), ## args)
#else
#define debug_printf(s, args...) do {} while(0)
#endif

#if ARCH == ARCH_HOST
#define debug_init() do { } while(0)
#define debug_putchar(ch) putchar(ch)
#elif defined(DEBUG_USE_SYSLOG)
#define debug_init() syslog_debug_init()
#define debug_putchar(ch) syslog_debug_put(ch, NULL)
#define debug_putstr(s) syslog_send(s)
#else /* not DEBUG_USE_SYSLOG */
#define debug_init() debug_init_uart()
#define debug_putchar(ch) debug_uart_put (ch, NULL)
#define debug_putstr(s) debug_uart_putstr(s)
#endif /* not DEBUG_USE_SYSLOG */

#else /* not DEBUG */
#define debug_init(...) do { } while(0)
#define debug_printf(s, args...) do {} while(0)
#define debug_putchar(...) do { } while(0)
#define debug_putstr(...) do { } while(0)
#endif /* not DEBUG */

/* prototypes */
void debug_init_uart(void);
void debug_process_uart(void);
int noinline debug_uart_put(char d, FILE * stream);
void noinline debug_uart_putstr(const char *);

/* Return a string of 0 and 1 expressing the value of the 8bit integer v.
 * Example: 00001000 */
char *debug_binary(uint8_t v);


#ifdef DEBUG_NTP_ADJUST
#define NTPADJDEBUG(a...)  debug_printf("ntpadj: " a)
#else
#define NTPADJDEBUG(a...)
#endif

#endif /* _DEBUG_H */
