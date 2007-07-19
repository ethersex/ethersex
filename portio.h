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

#ifndef _IO_H
#define _IO_H

#include "config.h"

#if defined(_ATMEGA644) || defined(_ATMEGA32)

#define IO_PORTS 4
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD}
#define IO_MASK_ARRAY { 0,      /* port a */                        \
                        0xff,   /* port b */                        \
                        _BV(PC0) | _BV(PC1), /* port c */           \
                        _BV(PD0) | _BV(PD1) | _BV(PD2) | _BV(PD3)   \
                      }

#else
#error "unknown CPU!"
#endif

/* prototypes */

/* update port information (PORT and DDR) from global status */
void portio_init(void);
void portio_update(void);

#endif /* _IO_H */
