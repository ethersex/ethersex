/*
 * Infrared-Multiprotokoll-Decoder 
 *
 * for additional information please
 * see http://www.mikrocontroller.net/articles/IRMP
 *
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "irmp.h"


#if defined(IRMP_SUPPORT_RECS80_PROTOCOL) || defined(IRMP_SUPPORT_RECS80EXT_PROTOCOL)
#define IRMP_HZ            20000	/* interrupts per second */
#elif defined(IRMP_SUPPORT_SIEMENS_PROTOCOL)
#define IRMP_HZ            15000	/* interrupts per second */
#else
#define IRMP_HZ            10000	/* interrupts per second */
#endif

#define MAX_OVERFLOW       255UL
#ifdef IRMP_USE_TIMER2
#if (F_CPU/IRMP_HZ) < MAX_OVERFLOW
#define HW_PRESCALER       1UL
#define HW_PRESCALER_MASK  _BV(CS20)
#elif (F_CPU/IRMP_HZ/8) < MAX_OVERFLOW
#define HW_PRESCALER       8UL
#define HW_PRESCALER_MASK  _BV(CS21)
#elif (F_CPU/IRMP_HZ/64) < MAX_OVERFLOW
#define HW_PRESCALER       64UL
#define HW_PRESCALER_MASK  _BV(CS21)|_BV(CS20)
#elif (F_CPU/IRMP_HZ/256) < MAX_OVERFLOW
#define HW_PRESCALER       256UL
#define HW_PRESCALER_MASK  _BV(CS22)
#elif (F_CPU/IRMP_HZ/1024) < MAX_OVERFLOW
#define HW_PRESCALER       1024UL
#define HW_PRESCALER_MASK  _BV(CS22)|_BV(CS00)
#else
#error F_CPU to large
#endif
#else
#if (F_CPU/IRMP_HZ) < MAX_OVERFLOW
#define HW_PRESCALER       1UL
#define HW_PRESCALER_MASK  _BV(CS00)
#elif (F_CPU/IRMP_HZ/8) < MAX_OVERFLOW
#define HW_PRESCALER       8UL
#define HW_PRESCALER_MASK  _BV(CS01)
#elif (F_CPU/IRMP_HZ/64) < MAX_OVERFLOW
#define HW_PRESCALER       64UL
#define HW_PRESCALER_MASK  _BV(CS01)|_BV(CS00)
#elif (F_CPU/IRMP_HZ/256) < MAX_OVERFLOW
#define HW_PRESCALER       256UL
#define HW_PRESCALER_MASK  _BV(CS02)
#elif (F_CPU/IRMP_HZ/1024) < MAX_OVERFLOW
#define HW_PRESCALER       1024UL
#define HW_PRESCALER_MASK  _BV(CS02)|_BV(CS00)
#else
#error F_CPU to large
#endif
#endif
#define SW_PRESCALER       ((F_CPU/HW_PRESCALER)/IRMP_HZ)

#ifdef IRMP_RX_LED
#ifdef IRMP_RX_LED_LOW_ACTIVE
#define IRMP_RX_LED_ON     IRMP_RX_LED_PORT &= ~_BV (IRMP_RX_LED_PIN)
#define IRMP_RX_LED_OFF    IRMP_RX_LED_PORT |= _BV (IRMP_RX_LED_PIN)
#else
#define IRMP_RX_LED_ON     IRMP_RX_LED_PORT |= _BV (IRMP_RX_LED_PIN)
#define IRMP_RX_LED_OFF    IRMP_RX_LED_PORT &= ~_BV (IRMP_RX_LED_PIN)
#endif
#else
#define IRMP_RX_LED_ON
#define IRMP_RX_LED_OFF
#endif

#ifdef IRMP_RX_LOW_ACTIVE
#define IRMP_RX_SPACE      _BV(IRMP_RX_PIN)
#define IRMP_RX_MARK       0
#else
#define IRMP_RX_SPACE      0
#define IRMP_RX_MARK       _BV(IRMP_RX_PIN)
#endif

#define FIFO_SIZE          8
#define FIFO_NEXT(x)       (((x)+1)&(FIFO_SIZE-1))


///////////////
#define irmp_ISR ir_rx_process
#define irmp_get_data ir_rx_get
#define irsnd_ISR ir_tx_process
#define irsnd_send_data ir_tx_put
#define F_INTERRUPTS   IRMP_HZ
#define IRMP_LOGGING   0
#define IRMP_USE_AS_LIB
#pragma push_macro("DEBUG")
#undef DEBUG
#include "irmp_lib.c"
#pragma pop_macro("DEBUG")
///////////////

typedef struct
{
  uint8_t read;
  uint8_t write;
  ir_data_t buffer[FIFO_SIZE];
} ir_fifo_t;


static uint16_t prescaler;
static ir_fifo_t ir_rx_fifo;

static const char proto_unknown[] PROGMEM = "unknown";
static const char proto_sircs[] PROGMEM = "SIRCS";
static const char proto_nec[] PROGMEM = "NEC";
static const char proto_samsung[] PROGMEM = "SAMSUNG";
static const char proto_matshushita[] PROGMEM = "MATSUSHITA";
static const char proto_kaseikyo[] PROGMEM = "KASEIKYO";
static const char proto_recs80[] PROGMEM = "RECS80";
static const char proto_rc5x[] PROGMEM = "RC5(x)";
static const char proto_denon[] PROGMEM = "DENON";
static const char proto_rc6[] PROGMEM = "RC6";
static const char proto_samsung32[] PROGMEM = "SAMSUNG32";
static const char proto_apple[] PROGMEM = "APPLE";
static const char proto_recs80ext[] PROGMEM = "RECS80EXT";
static const char proto_nubert[] PROGMEM = "NUBERT";
static const char proto_bang_olufsen[] PROGMEM = "BANG&OLUFSEN";
static const char proto_grundig[] PROGMEM = "GRUNDIG";
static const char proto_nokia[] PROGMEM = "NOKIA";
static const char proto_siemens[] PROGMEM = "SIEMENS";
static const char proto_fdc[] PROGMEM = "FDC";
static const char proto_rccar[] PROGMEM = "RCCAR";
static const char proto_jvc[] PROGMEM = "JVC";
static const char proto_rc6a[] PROGMEM = "RC6A";

const PGM_P proto_names[] PROGMEM = {
  proto_unknown,
  proto_sircs,
  proto_nec,
  proto_samsung,
  proto_matshushita,
  proto_kaseikyo,
  proto_recs80,
  proto_rc5x,
  proto_denon,
  proto_rc6,
  proto_samsung32,
  proto_apple,
  proto_recs80ext,
  proto_nubert,
  proto_bang_olufsen,
  proto_grundig,
  proto_nokia,
  proto_siemens,
  proto_fdc,
  proto_rccar,
  proto_jvc,
  proto_rc6a
};


void
irmp_init (void)
{
  /* configure TSOP input, disable pullup */
  DDR_CONFIG_IN (IRMP_RX);
  PIN_CLEAR (IRMP_RX);

#ifdef IRMP_RX_LED
  DDR (IRMP_RX_LED_PORT) &= ~_BV (IRMP_RX_LED_PIN);
  IRMP_RX_LED_PORT |= _BV (IRMP_RX_LED_PIN);
#endif

  /* init timer0/2 to expire after 1000/IRMP_HZ ms */
  prescaler = (uint16_t) IRMP_HZ;
#ifdef IRMP_USE_TIMER2
  TCCR2 = HW_PRESCALER_MASK;
  OCR2 = SW_PRESCALER - 1;
  TCNT2 = 0;
  _TIMSK_TIMER2 |= _BV (OCIE0);	/* enable interrupt */
#else
  TCCR0 = HW_PRESCALER_MASK;
  OCR0 = SW_PRESCALER - 1;
  TCNT0 = 0;
  _TIMSK_TIMER0 |= _BV (OCIE0);	/* enable interrupt */
#endif
}


uint8_t
irmp_read (ir_data_t * ir_data_p)
{
  if (ir_rx_fifo.read == ir_rx_fifo.write)
    return 0;

  *ir_data_p = ir_rx_fifo.buffer[ir_rx_fifo.read =
				 FIFO_NEXT (ir_rx_fifo.read)];

#ifdef DEBUG_IRMP
  printf_P (PSTR ("IRMP: proto "));
  printf_P ((const char *) pgm_read_word (&proto_names[ir_data_p->protocol]));
  printf_P (PSTR (", address %04x, command %04x, repeat %d\n"),
	    ir_data_p->address, ir_data_p->command,
	    ir_data_p->flags & IRMP_FLAG_REPETITION ? 1 : 0);
#endif
  return 1;
}


void
irmp_process (void)
{
  ir_data_t ir_data;
  (void) irmp_read (&ir_data);
}


#ifdef IRMP_USE_TIMER2
ISR (TIMER1_COMP_vect)
#else
ISR (TIMER0_COMP_vect)
#endif
{
  uint8_t data = PIN_HIGH (IRMP_RX) & PIN_BV (IRMP_RX);
#ifdef IRMP_RX_LED
  if (data == IRMP_RX_MARK)
    IRMP_RX_LED_ON;
  else
    IRMP_RX_LED_OFF;
#endif

  if (ir_rx_process (data) != 0)
    {
      uint8_t tmphead = FIFO_NEXT (ir_rx_fifo.write);
      if (tmphead != ir_rx_fifo.read)
	{
	  if (ir_rx_get (&ir_rx_fifo.buffer[tmphead]))
	    ir_rx_fifo.write = tmphead;
	}
    }

  if (--prescaler == 0)
    prescaler = (uint16_t) IRMP_HZ;
#if (F_CPU/HW_PRESCALER) % IRMP_HZ
  if (prescaler <= (F_CPU / HW_PRESCALER) % IRMP_HZ)
#ifdef IRMP_USE_TIMER2
    OCR2 += SW_PRESCALER + 1;	/* um 1 Takt längere Periode um
				   den Rest abzutragen */
#else
    OCR0 += SW_PRESCALER + 1;
#endif
  else
#endif
#ifdef IRMP_USE_TIMER2
    OCR2 += SW_PRESCALER;	/* kurze Periode */
#else
    OCR0 += SW_PRESCALER;
#endif
}

/*
  -- Ethersex META --
  header(hardware/ir/irmp/irmp.h)
  mainloop(irmp_process)
  init(irmp_init)
*/
