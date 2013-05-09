/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include "config.h"
#if ARCH != ARCH_HOST

#include <stdint.h>
#include <avr/interrupt.h>
#include "core/bit-macros.h"

#ifndef _USART_H
#define _USART_H

#ifndef USE_USART
#define USE_USART
#endif

#define _usart_cat_(a,b) a ## b
#define _usart_cat(a,b) _usart_cat_(a,b)

#if !defined(UDR0) || !defined(USE_USART)
#undef USE_USART
#define USE_USART
#endif

#ifndef USART_TX_vect
#  if defined(USART0_TX_vect)
#    define USART_TX_vect USART0_TX_vect
#    define USART_RX_vect USART0_RX_vect
#  elif defined(USART0_TXC_vect)
#    define USART_TX_vect USART0_TXC_vect
#    define USART_RX_vect USART0_RXC_vect
#  elif defined(USART_TXC_vect)
#    define USART_TX_vect USART_TXC_vect
#    define USART_RX_vect USART_RXC_vect
#  else
#    error "I hate avr-libc"
#  endif
#endif



// Sugar for the avr-libc
#if !defined(USART0_TX0_vect) && defined(USART0_TXC_vect)
#  define USART0_TX_vect USART0_TXC_vect
#  define USART1_TX_vect USART1_TXC_vect
#  define USART0_RX_vect USART0_RXC_vect
#  define USART1_RX_vect USART1_RXC_vect
#endif

#define usart(a, ...) _usart_cat(a, _usart_cat(USE_USART, __VA_ARGS__))

/* We love the preprocessor */
#if defined(URSEL) || defined(URSEL0)
#define _BV_URSEL _BV(usart(URSEL))
#else
#define _BV_URSEL 0
#endif

/* If the Baudrate isn't set by the module which is using usart.h */
#ifndef BAUD
#define BAUD 9600
#endif

/* We use setbaud.h from the avr-libc */
#include <util/setbaud.h>
#include <util/atomic.h>

/* This is used in generate_usart_init() */
#if USE_2X
#define USART_2X() usart(UCSR,A) |= (1 << usart(U2X))
#else
#define USART_2X() usart(UCSR,A) &= ~(1 << usart(U2X))
#endif


/* init the usart module */
#define generate_usart_init() \
static void \
usart_init(void) \
{\
  /* The ATmega644 datasheet suggests to clear the global \
   * interrupt flags on initialization ... */\
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) \
  { \
    usart(UBRR,H) = UBRRH_VALUE; \
    usart(UBRR,L) = UBRRL_VALUE; \
    /* set mode 8N1: 8 bits, 1 stop, no parity, asynchronous usart \
     * and set URSEL, if present, */ \
    usart(UCSR,C) = _BV(usart(UCSZ,0)) | _BV(usart(UCSZ,1)) | _BV_URSEL; \
    /* Enable the RX interrupt and receiver and transmitter */ \
    usart(UCSR,B) |= _BV(usart(TXEN)) | _BV(usart(RXEN)) | _BV(usart(RXCIE)); \
    /* Set or not set the 2x mode */ \
    USART_2X(); \
  } \
}


  /* init the usart module ( 8N2 )*/
#define generate_usart_init_8N2() \
static void \
usart_init(void) \
{\
  /* The ATmega644 datasheet suggests to clear the global\
   * interrupt flags on initialization ... */\
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) \
  { \
    usart(UBRR,H) = UBRRH_VALUE; \
    usart(UBRR,L) = UBRRL_VALUE; \
    /* set mode 8N2: 8 bits, 2 stop, no parity, asynchronous usart \
     * and set URSEL, if present, */ \
    usart(UCSR,C) =  _BV(usart(USBS)) | (3 << (usart(UCSZ,0))) | _BV_URSEL; \
    /* Do *not* enable the RX interrupt and receiver and transmitter */\
    usart(UCSR,B) = 0; \
    /* Set or not set the 2x mode */ \
    USART_2X(); \
  } \
}

#endif /* _USART_H */

#endif /* ARCH != ARCH_HOST */
