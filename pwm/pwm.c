/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "../config.h"
#include "../debug.h"
#include "../ecmd_parser/ecmd.h"


#ifdef PWM_SUPPORT
#include "pwm.h"

#define SOUNDFREQ 8000

uint16_t pwmbytecounter = 0;
uint8_t pwmrepeat = 0;

//Timer2 Interrupt
ISR (TIMER2_OVF_vect)
{
	pwmbytecounter++;
	uint8_t s = pgm_read_byte(&pwmsound[pwmbytecounter]);
#ifdef DEBUG_PWM
//    	debug_printf("PWM sound %x at pos %i, repeat %i\n",s, pwmbytecounter, pwmrepeat);
#endif
	TCNT2 = 65535 - (MCU/64/SOUNDFREQ);
	OCR2A = s;
	if(pwmbytecounter > pwmmaxsoundbytes)
	{
		pwmbytecounter = 0;
		pwmrepeat++;
	}
	if (pwmrepeat >10) {
		pwmrepeat = 0;
		pwm_stop();
	}
}

void
pwm_wav_init(void)
{
#ifdef DEBUG_PWM
    	debug_printf("PWM wav init: \n");
#endif
	//Set TIMER2 (PWM OC2 Pin = PD7)
	DDRD |= (1<<7);
	TCCR2A |= (1<<WGM21|1<<WGM20|1<<COM2A1|1<<CS20);
	OCR2A = 128;
	
	//Set TIMER2 
	TIMSK2 |= (1 << TOIE2);
	//Prescaler 64
	TCCR2B = (1<<CS20 | 1<<CS21);
	//SYSCLK define in usart.h
	TCNT2 = 65535 - (MCU/64/SOUNDFREQ);
}

void 
pwm_stop()
{
#ifdef DEBUG_PWM
    	debug_printf("PWM stop: \n");
#endif
    TIMSK2 &= ~_BV(TOIE2);

    // Disable playback per-sample interrupt.
    TIMSK2 &= ~_BV(OCIE2A);

    // Disable the per-sample timer completely.
    TCCR1A &= ~_BV(CS20);

    // Disable the PWM timer.
    TCCR2B &= ~_BV(CS20);
}

#endif /*PWM_SUPPORT*/
