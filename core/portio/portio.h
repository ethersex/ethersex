/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _IO_H
#define _IO_H

#include "config.h"
#include <stdint.h>

#if defined(_ATMEGA8) || defined(_ATMEGA88) || defined(_ATMEGA168) \
    || defined(_ATMEGA168P) || defined(_ATMEGA328P)
#define IO_HARD_PORTS 3
#define IO_DDR_ARRAY {&DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTB, &PORTC, &PORTD}
#define IO_PIN_ARRAY { &PINB, &PINC, &PIND}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D   /* port d from pinning.m4 */ \
                       }

/* ATMega644 (644p)| ATMega32*/
#elif defined(_ATMEGA644) || defined(_ATMEGA32) || defined(_ATMEGA162) \
    || defined(_ATMEGA16) || defined(_ATMEGA1284P)


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

/* ATMega64 | ATMega128 */
#elif defined(_ATMEGA64) || defined(_ATMEGA128)


#define IO_HARD_PORTS 6
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD, &DDRE, &DDRF}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF}
#define IO_PIN_ARRAY {&PINA, &PINB, &PINC, &PIND, &PINE, &PINF}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_A , /* port a from pinning.m4 */ \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D , /* port d from pinning.m4 */ \
                        255 - PORTIO_MASK_E , /* port e from pinning.m4 */ \
                        255 - PORTIO_MASK_F   /* port f from pinning.m4 */ \
                       }

/* ATMega640 | ATMega1280 | ATMega2560 */
#elif defined(_ATMEGA2560)


#define IO_HARD_PORTS 11
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD, &DDRE, &DDRF, &DDRG, &DDRH, &DDRJ, &DDRK, &DDRL}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF, &PORTG, &PORTH, &PORTJ, &PORTK, &PORTL}
#define IO_PIN_ARRAY {&PINA, &PINB, &PINC, &PIND, &PINE, &PINF, &PING, &PINH, &PINJ, &PINK, &PINL}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_A , /* port a from pinning.m4 */ \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D , /* port d from pinning.m4 */ \
                        255 - PORTIO_MASK_E , /* port e from pinning.m4 */ \
                        255 - PORTIO_MASK_F , /* port f from pinning.m4 */ \
                        255 - PORTIO_MASK_G , /* port g from pinning.m4 */ \
                        255 - PORTIO_MASK_H , /* port h from pinning.m4 */ \
                        255 - PORTIO_MASK_J , /* port j from pinning.m4 */ \
                        255 - PORTIO_MASK_K , /* port k from pinning.m4 */ \
                        255 - PORTIO_MASK_L   /* port l from pinning.m4 */ \
                       }

/* ATMega169 | AT90CAN128 | AT90CAN64 | AT90CAN32 */
#elif defined(_ATMEGA169) || defined(_AT90CAN128) || defined(_AT90CAN64) || defined(_AT90CAN32)


#define IO_HARD_PORTS 7
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD, &DDRE, &DDRF, &DDRG}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD, &PORTE, &PORTF, &PORTG}
#define IO_PIN_ARRAY {&PINA, &PINB, &PINC, &PIND, &PINE, &PINF, &PING}
#define IO_MASK_ARRAY {                                              \
                        255 - PORTIO_MASK_A , /* port a from pinning.m4 */ \
                        255 - PORTIO_MASK_B , /* port b from pinning.m4 */ \
                        255 - PORTIO_MASK_C , /* port c from pinning.m4 */ \
                        255 - PORTIO_MASK_D , /* port d from pinning.m4 */ \
                        255 - PORTIO_MASK_E , /* port e from pinning.m4 */ \
                        255 - PORTIO_MASK_F , /* port f from pinning.m4 */ \
                        255 - PORTIO_MASK_G   /* port g from pinning.m4 */ \
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
void np_simple_init(void);
  
#endif /* _IO_H */
