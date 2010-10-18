/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) by David Gräff <david.graeff@web.de>
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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/periodic.h"
#include "core/debug.h"

#ifdef BOOTLOADER_SUPPORT
uint8_t bootload_delay = CONF_BOOTLOAD_DELAY;
#endif

extern volatile uint8_t newtick;
uint8_t milliticks;

void periodic_init(void) {

#if CLOCK_DIVIDER == 1024
#define CLOCKSCALE _BV(CS12) | _BV(CS10)
#else  // 256
#define	CLOCKSCALE  _BV(CS12)
#endif

#ifdef CLOCK_CPU_SUPPORT
	/* init timer1 to expire after ~20ms, with Normal */
	TCCR1B = CLOCKSCALE; 
	TCNT1 = 65536-CLOCK_SECONDS;
	OCR1A = 65536-CLOCK_SECONDS+CLOCK_TICKS;
	_TIMSK_TIMER1 |= _BV(OCIE1A)|_BV(TOIE1);
#else
	/* init timer1 to expire after ~20ms, with CTC enabled */
	TCCR1B = _BV(WGM12) | CLOCKSCALE;
	OCR1A = CLOCK_TICKS - 1;
	_TIMSK_TIMER1 |= _BV(OCIE1A);
	NTPADJDEBUG ("configured OCR1A to %d\n", OCR1A);
#endif
}

ISR(TIMER1_COMPA_vect)
{
#ifdef CLOCK_CPU_SUPPORT
	OCR1A += CLOCK_TICKS;
#endif
	newtick = 1;
        if(++milliticks >= 50) milliticks=0;
}

/*
 -- Ethersex META --
 header(core/periodic.h)
 init(periodic_init)
 */
