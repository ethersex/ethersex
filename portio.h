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

#ifdef _ATMEGA8
#define IO_HARD_PORTS 3
#define IO_DDR_ARRAY {&DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTB, &PORTC, &PORTD}
#define IO_PIN_ARRAY { &PINB, &PINC, &PIND}
/* FIXME portio not really supported. */
#define IO_MASK_ARRAY { 0, 0, 0 }                                          \

/* ATMega644 | ATMega32 */
#elif defined(_ATMEGA644) || defined(_ATMEGA32)

#if defined(HD44780_SUPPORT) && !defined(HD44780_USE_PORTC)
    #define PORTA_MASK (_BV(HD44780_RS) | \
                        _BV(HD44780_RW) | \
                        _BV(HD44780_EN) | \
                        _BV(HD44780_D4) | \
                        _BV(HD44780_D5) | \
                        _BV(HD44780_D6) | \
                        _BV(HD44780_D7))
    #define PORTC_MASK 0
#elif defined(HD44780_SUPPORT) && defined(HD44780_USE_PORTC)
    #define PORTA_MASK 0
    #define PORTC_MASK (_BV(HD44780_RS) | \
                        _BV(HD44780_RW) | \
                        _BV(HD44780_EN) | \
                        _BV(HD44780_D4) | \
                        _BV(HD44780_D5) | \
                        _BV(HD44780_D6) | \
                        _BV(HD44780_D7))
#else
    #define PORTA_MASK 0
    #define PORTC_MASK 0
#endif

#ifdef STELLA_SUPPORT
#define STELLA_MASK (_BV(PD5) | _BV(PD6) | _BV(PD7))
#else
#define STELLA_MASK 0
#endif 

#ifdef RFM12_SUPPORT
    #define RFM12_PORTC_MASK (_BV(SPI_CS_RFM12))
#else
    #define RFM12_PORTC_MASK 0
#endif

#define IO_HARD_PORTS 4
#define IO_DDR_ARRAY {&DDRA, &DDRB, &DDRC, &DDRD}
#define IO_PORT_ARRAY {&PORTA, &PORTB, &PORTC, &PORTD}
#define IO_PIN_ARRAY {&PINA, &PINB, &PINC, &PIND}
#define IO_MASK_ARRAY {                                             \
                        0 | PORTA_MASK,              /* port a */   \
                        0xff,                        /* port b */   \
                        _BV(PC0) | _BV(PC1) | PORTC_MASK            \
                         | RFM12_PORTC_MASK,         /* port c */   \
                        _BV(PD0) | _BV(PD1) | _BV(PD2) | _BV(PD3)   \
			  | STELLA_MASK				    \
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
