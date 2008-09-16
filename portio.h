/* vim:fdm=marker et ai
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
#include <stdint.h>

#if defined(_ATMEGA8) || defined(_ATMEGA88)
#define IO_HARD_PORTS 3
#define IO_DDR_ARRAY {&DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTB, &PORTC, &PORTD}
#define IO_PIN_ARRAY { &PINB, &PINC, &PIND}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D   /* port d from pinning.m4 */ \
                       }

/* ATMega644 | ATMega32 */
#elif defined(_ATMEGA644) || defined(_ATMEGA32) || defined(_ATMEGA162)


#define IO_HARD_PORTS 4
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD}
#define IO_PIN_ARRAY {&PINA, &PINB, &PINC, &PIND}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_A , /* port a from pinning.m4 */ \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D   /* port d from pinning.m4 */ \
                       }

#else
#error "unknown CPU!"
#endif

/* Yeah we like preprocessor macros */
#if defined(HC595_SUPPORT) && (!defined(HC165_SUPPORT))
  #define IO_PORTS (IO_HARD_PORTS + HC595_REGISTERS)
#elif (!defined(HC595_SUPPORT)) && defined(HC165_SUPPORT)
  #define IO_PORTS (IO_HARD_PORTS + HC165_REGISTERS)
#elif defined(HC595_SUPPORT) && defined(HC165_SUPPORT)
  #if HC595_REGISTERS > HC165_REGISTERS
    #define IO_PORTS (IO_HARD_PORTS + HC595_REGISTERS)
  #else
    #define IO_PORTS (IO_HARD_PORTS + HC165_REGISTERS)
  #endif
#else
  #define IO_PORTS IO_HARD_PORTS
#endif



typedef struct  {
  uint8_t mask;
  uint8_t (*read_port)(uint8_t port);
  uint8_t (*write_port)(uint8_t port, uint8_t data);
  uint8_t (*read_ddr)(uint8_t port);
  uint8_t (*write_ddr)(uint8_t port, uint8_t data);
  uint8_t (*read_pin)(uint8_t port);
} virtual_port_t;

/* Only if not included by portio.c */
extern virtual_port_t vport[];

/* prototypes */

/* update port information (PORT and DDR) from global status */
void portio_init(void);
  
#endif /* _IO_H */
