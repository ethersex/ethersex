/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) by Ulrich Radig <mail@ulrichradig.de>
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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "pwm_wav.h"
#include "ethersex_wav.h"
#include "protocols/ecmd/ecmd-base.h"

//Sound Daten (got by MegaLOG of Ulrich Radig (see Radig webmodul ))

#define SOUNDFREQ 8000
#define SOUNDDIVISOR (F_CPU/64/SOUNDFREQ)

uint16_t pwmbytecounter = 0;

//Timer2 Interrupt
ISR (TIMER0_OVF_vect)
{
	uint8_t s = pgm_read_byte(&pwmsound[pwmbytecounter]);
#ifdef DEBUG_PWM
    	if (pwmbytecounter < 10 || ((pwmbytecounter % 1000) == 0) ) debug_printf("PWM sound %x at pos %i\n",s, pwmbytecounter);
#endif
	TCNT0 = 255 - SOUNDDIVISOR;
	OCR2A = s;
	pwmbytecounter++;
	if(pwmbytecounter > sizeof(pwmsound))
	{
		pwmbytecounter = 0;
		pwm_stop();
	}
}

void
pwm_wav_init(void)
{
	pwmbytecounter = 0;
#ifdef DEBUG_PWM
    	debug_printf("PWM wav init, size: %i, %i Hz \n", sizeof(pwmsound), SOUNDFREQ );
#endif
	//Set TIMER2 (PWM OC2 Pin = PD7)
	DDRD |= (1<<7);
	TCCR2A |= (1<<WGM21|1<<WGM20|1<<COM2A1);
	TCCR2B |= (1<<CS20);
	OCR2A = 128;

	//Set TIMER0
	TIMSK0 |= (1 << TOIE0);
	TCCR0B = (1<<CS00|1<<CS01) ;
	TCNT0 = 255 - SOUNDDIVISOR;
}

void
pwm_stop()
{
#ifdef DEBUG_PWM
    	debug_printf("PWM stop\n");
#endif
	// timer 2 stop
	TCCR2B = 0;

	// timer 0 stop
	TCCR0B = 0 ;

}

int16_t
parse_cmd_pwm_wav_play(char *cmd, char *output, uint16_t len)
{
    pwm_wav_init();
    return ECMD_FINAL_OK;
}

int16_t
parse_cmd_pwm_wav_stop(char *cmd, char *output, uint16_t len)
{
    pwm_stop();
    return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block([[Sound]]/WAV support)
  ecmd_feature(pwm_wav_play, "pwm wav", , Play wav)
  ecmd_feature(pwm_wav_stop, "pwm stop", , Stop wav)
*/
