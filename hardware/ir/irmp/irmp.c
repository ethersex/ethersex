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


#if defined(IRMP_SUPPORT_RE_CS80_PROTOCOL) || defined(IRMP_SUPPORT_RE_CS80EXT_PROTOCOL)
#define IRMP_HZ            20000	/* interrupts per second */
#elif defined(IRMP_SUPPORT_SIEMENS_PROTOCOL)
#define IRMP_HZ            15000
#else
#define IRMP_HZ            10000
#endif

#define MAX_OVERFLOW       255UL
#ifdef IRMP_USE_TIMER2
#if (F_CPU/IRMP_HZ) < MAX_OVERFLOW
#define HW_PRESCALER       1UL
#define HW_PRESCALER_MASK  _BV(_CS20)
#elif (F_CPU/IRMP_HZ/8) < MAX_OVERFLOW
#define HW_PRESCALER       8UL
#define HW_PRESCALER_MASK  _BV(_CS21)
#elif (F_CPU/IRMP_HZ/64) < MAX_OVERFLOW
#define HW_PRESCALER       64UL
#define HW_PRESCALER_MASK  _BV(_CS21)|_BV(_CS20)
#elif (F_CPU/IRMP_HZ/256) < MAX_OVERFLOW
#define HW_PRESCALER       256UL
#define HW_PRESCALER_MASK  _BV(_CS22)
#elif (F_CPU/IRMP_HZ/1024) < MAX_OVERFLOW
#define HW_PRESCALER       1024UL
#define HW_PRESCALER_MASK  _BV(_CS22)|_BV(_CS00)
#else
#error F_CPU to large
#endif
#else
#if (F_CPU/IRMP_HZ) < MAX_OVERFLOW
#define HW_PRESCALER       1UL
#define HW_PRESCALER_MASK  _BV(_CS00)
#elif (F_CPU/IRMP_HZ/8) < MAX_OVERFLOW
#define HW_PRESCALER       8UL
#define HW_PRESCALER_MASK  _BV(_CS01)
#elif (F_CPU/IRMP_HZ/64) < MAX_OVERFLOW
#define HW_PRESCALER       64UL
#define HW_PRESCALER_MASK  _BV(_CS01)|_BV(_CS00)
#elif (F_CPU/IRMP_HZ/256) < MAX_OVERFLOW
#define HW_PRESCALER       256UL
#define HW_PRESCALER_MASK  _BV(_CS02)
#elif (F_CPU/IRMP_HZ/1024) < MAX_OVERFLOW
#define HW_PRESCALER       1024UL
#define HW_PRESCALER_MASK  _BV(_CS02)|_BV(_CS00)
#else
#error F_CPU to large
#endif
#endif
#define SW_PRESCALER       ((uint8_t)((F_CPU/HW_PRESCALER)/IRMP_HZ))

#ifdef IRMP_RX_LED
#ifdef IRMP_RX_LED_LOW_ACTIVE
#define IRMP_RX_LED_ON     PIN_CLEAR(STATUSLED_RX)
#define IRMP_RX_LED_OFF    PIN_SET(STATUSLED_RX)
#else
#define IRMP_RX_LED_ON     PIN_SET(STATUSLED_RX)
#define IRMP_RX_LED_OFF    PIN_CLEAR(STATUSLED_RX)
#endif
#else
#define IRMP_RX_LED_ON
#define IRMP_RX_LED_OFF
#endif

#ifdef IRMP_RX_LOW_ACTIVE
#define IRMP_RX_SPACE      PIN_BV(IRMP_RX)
#define IRMP_RX_MARK       0
#else
#define IRMP_RX_SPACE      0
#define IRMP_RX_MARK       PIN_BV(IRMP_RX)
#endif

#define FIFO_SIZE          8
#define FIFO_NEXT(x)       (((x)+1)&(FIFO_SIZE-1))


///////////////
#pragma push_macro("F_INTERRUPTS")
#define F_INTERRUPTS IRMP_HZ
#pragma push_macro("DEBUG")
#undef DEBUG
#define irmp_ISR irmp_rx_process
#define irmp_get_data irmp_rx_get
#define IRMP_LOGGING 0
#define IRMP_USE_AS_LIB
#define IRMP_DATA irmp_data_t
#include "irmp_lib.c"
#ifdef IRSND_SUPPORT
#define irsnd_ISR irmp_tx_process
#define irsnd_on irmp_tx_on
#define irsnd_off irmp_tx_off
#define irsnd_send_data irmp_tx_put
#define irsnd_set_freq irmp_tx_set_freq
#define IRSND_USE_AS_LIB
static void irmp_tx_on (void);
static void irmp_tx_off (void);
static void irmp_tx_set_freq (uint8_t);
#include "irsnd_lib.c"
#endif
#pragma pop_macro("DEBUG")
#pragma pop_macro("F_INTERRUPTS")
///////////////

typedef struct
{
  uint8_t read;
  uint8_t write;
  irmp_data_t buffer[FIFO_SIZE];
} irmp_fifo_t;

static irmp_fifo_t irmp_rx_fifo;
#ifdef IRSND_SUPPORT
static irmp_fifo_t irmp_tx_fifo;
#endif

#ifdef DEBUG_IRMP
static const char proto_unknown[] PROGMEM = "unknown";
static const char proto_sircs[] PROGMEM = "SIR_CS";
static const char proto_nec[] PROGMEM = "NEC";
static const char proto_samsung[] PROGMEM = "SAMSUNG";
static const char proto_matshushita[] PROGMEM = "MATSUSHITA";
static const char proto_kaseikyo[] PROGMEM = "KASEIKYO";
static const char proto_recs80[] PROGMEM = "RE_CS80";
static const char proto_rc5x[] PROGMEM = "RC5(x)";
static const char proto_denon[] PROGMEM = "DENON";
static const char proto_rc6[] PROGMEM = "RC6";
static const char proto_samsung32[] PROGMEM = "SAMSUNG32";
static const char proto_apple[] PROGMEM = "APPLE";
static const char proto_recs80ext[] PROGMEM = "RE_CS80EXT";
static const char proto_nubert[] PROGMEM = "NUBERT";
static const char proto_bang_olufsen[] PROGMEM = "BANG&OLUFSEN";
static const char proto_grundig[] PROGMEM = "GRUNDIG";
static const char proto_nokia[] PROGMEM = "NOKIA";
static const char proto_siemens[] PROGMEM = "SIEMENS";
static const char proto_fdc[] PROGMEM = "FDC";
static const char proto_rccar[] PROGMEM = "RCCAR";
static const char proto_jvc[] PROGMEM = "JVC";
static const char proto_rc6a[] PROGMEM = "RC6A";
static const char proto_nikon[] PROGMEM = "NIKON";

const PGM_P irmp_proto_names[] PROGMEM = {
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
  proto_rc6a,
  proto_nikon
};
#endif


void
irmp_init (void)
{
  /* configure TSOP input, disable pullup */
  DDR_CONFIG_IN (IRMP_RX);
  PIN_CLEAR (IRMP_RX);

#ifdef IRMP_RX_LED
  DDR_CONFIG_OUT (STATUSLED_RX);
  IRMP_RX_LED_OFF;
#endif

  /* init timer0/2 to expire after 1000/IRMP_HZ ms */
#ifdef IRMP_USE_TIMER2
  _TCCR2_PRESCALE = HW_PRESCALER_MASK;
  _OUTPUT_COMPARE_REG2 = SW_PRESCALER - 1;
  _TCNT2 = 0;
  _TIMSK_TIMER2 |= _BV (_OUTPUT_COMPARE_IE2);	/* enable interrupt */
#else
  _TCCR0_PRESCALE = HW_PRESCALER_MASK;
  _OUTPUT_COMPARE_REG0 = SW_PRESCALER - 1;
  _TCNT0 = 0;
  _TIMSK_TIMER0 |= _BV (_OUTPUT_COMPARE_IE0);	/* enable interrupt */
#endif

#ifdef IRSND_SUPPORT
  PIN_CLEAR (IRMP_TX);
  DDR_CONFIG_OUT (IRMP_TX);
#ifdef IRMP_USE_TIMER2
  _TCCR0_PRESCALE = _BV (_WGM01) | _BV (_CS00);	/* CTC mode, 0x01, start Timer 0, no prescaling */
#else
  _TCCR2_PRESCALE = _BV (_WGM21) | _BV (_CS20);	/* CTC mode, 0x01, start Timer 2, no prescaling */
#endif
  irmp_tx_set_freq (IRSND_FREQ_36_KHZ);	/* default frequency */
#endif
}


uint8_t
irmp_read (irmp_data_t * irmp_data_p)
{
  if (irmp_rx_fifo.read == irmp_rx_fifo.write)
    return 0;

  *irmp_data_p = irmp_rx_fifo.buffer[irmp_rx_fifo.read =
				     FIFO_NEXT (irmp_rx_fifo.read)];

#ifdef DEBUG_IRMP
  printf_P (PSTR ("IRMP RX: proto %02" PRId8 " "), irmp_data_p->protocol);
  printf_P ((const char *)
	    pgm_read_word (&irmp_proto_names[irmp_data_p->protocol]));
  printf_P (PSTR
	    (", address %04" PRIX16 ", command %04" PRIX16 ", flags %02" PRIX8
	     "\n"), irmp_data_p->address, irmp_data_p->command,
	    irmp_data_p->flags);
#endif
  return 1;
}


#ifdef IRSND_SUPPORT

static void
irmp_tx_on (void)
{
  if (!irsnd_is_on)
    {
#ifdef IRMP_USE_TIMER2
      _TCCR0_PRESCALE |= _BV (_COM00) | _BV (_WGM01);
#else
      _TCCR2_PRESCALE |= _BV (_COM20) | _BV (_WGM21);
#endif
      irsnd_is_on = TRUE;
    }
}


static void
irmp_tx_off (void)
{
  if (irsnd_is_on)
    {
#ifdef IRMP_USE_TIMER2
      _TCCR0_PRESCALE &= ~_BV (_COM00);
#else
      _TCCR2_PRESCALE &= ~_BV (_COM20);
#endif
      PIN_CLEAR (IRMP_TX);
      irsnd_is_on = FALSE;
    }
}


static void
irmp_tx_set_freq (uint8_t freq)
{
#ifdef IRMP_USE_TIMER2
  _OUTPUT_COMPARE_REG0 = freq;
#else
  _OUTPUT_COMPARE_REG2 = freq;
#endif
}


void
irmp_write (irmp_data_t * irmp_data_p)
{
#ifdef DEBUG_IRMP
  printf_P (PSTR ("IRMP TX: proto %02" PRId8 " "), irmp_data_p->protocol);
  printf_P ((const char *)
	    pgm_read_word (&irmp_proto_names[irmp_data_p->protocol]));
  printf_P (PSTR
	    (", address %04" PRIX16 ", command %04" PRIX16 ", flags %02" PRIX8
	     "\n"), irmp_data_p->address, irmp_data_p->command,
	    irmp_data_p->flags);
#endif

  uint8_t tmphead = FIFO_NEXT (irmp_tx_fifo.write);

  while (tmphead == *(volatile uint8_t *) &irmp_tx_fifo.read)
    _delay_ms (10);

  irmp_tx_fifo.buffer[tmphead] = *irmp_data_p;
  irmp_tx_fifo.write = tmphead;
}

#endif


#ifdef IRMP_USE_TIMER2
ISR (_VECTOR_OUTPUT_COMPARE2)
#else
ISR (_VECTOR_OUTPUT_COMPARE0)
#endif
{
#ifdef IRMP_USE_TIMER2
  _OUTPUT_COMPARE_REG2 += SW_PRESCALER;
#else
  _OUTPUT_COMPARE_REG0 += SW_PRESCALER;
#endif
  uint8_t data = PIN_HIGH (IRMP_RX) & PIN_BV (IRMP_RX);

#ifdef IRSND_SUPPORT
  if (irmp_tx_process () == 0)
    {
#endif

      if (data == IRMP_RX_MARK)
	{
	  IRMP_RX_LED_ON;
	}
      else
	{
	  IRMP_RX_LED_OFF;
	}

      if (irmp_rx_process (data) != 0)
	{
	  uint8_t tmphead = FIFO_NEXT (irmp_rx_fifo.write);
	  if (tmphead != irmp_rx_fifo.read)
	    {
	      if (irmp_rx_get (&irmp_rx_fifo.buffer[tmphead]))
		irmp_rx_fifo.write = tmphead;
	    }
	}

#ifdef IRSND_SUPPORT
      if (irmp_tx_fifo.read != irmp_tx_fifo.write)
	irmp_tx_put (&irmp_tx_fifo.buffer[irmp_tx_fifo.read =
					  FIFO_NEXT (irmp_tx_fifo.read)], 0);
    }
#endif
}

/*
  -- Ethersex META --
  header(hardware/ir/irmp/irmp.h)
  init(irmp_init)
*/
