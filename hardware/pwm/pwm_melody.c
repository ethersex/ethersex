/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * 20030116 - 1.0 - Created- LHM
 * Copyright (c) 2003, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "pwm_common.h"
#include "pwm_melody.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef ENTCHEN_PWM_MELODY_SUPPORT
 #include "entchen.h"
#endif /* ENTCHEN_PWM_MELODY_SUPPORT */

#ifdef TETRIS_PWM_MELODY_SUPPORT
 #include "tetris.h"
#endif /* TETRIS_PWM_MELODY_SUPPORT */

uint8_t pwm_melody_tone=0;
uint16_t pwm_melody_i=0;
volatile uint16_t pwm_melody_scale=523;

// simple sinus in 255 values
const uint8_t sinewave[1][256] PROGMEM=
{
{
0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
0xb0,0xb3,0xb6,0xb9,0xbc,0xbf,0xc1,0xc4,0xc7,0xc9,0xcc,0xce,0xd1,0xd3,0xd5,0xd8,
0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xed,0xef,0xf0,0xf2,0xf3,0xf5,
0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfd,0xfc,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,
0xf6,0xf5,0xf3,0xf2,0xf0,0xef,0xed,0xec,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
0xda,0xd8,0xd5,0xd3,0xd1,0xce,0xcc,0xc9,0xc7,0xc4,0xc1,0xbf,0xbc,0xb9,0xb6,0xb3,
0xb0,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x63,0x60,0x5d,0x5a,0x57,0x54,0x51,
0x4f,0x4c,0x49,0x46,0x43,0x40,0x3e,0x3b,0x38,0x36,0x33,0x31,0x2e,0x2c,0x2a,0x27,
0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x12,0x10,0x0f,0x0d,0x0c,0x0a,
0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x0a,0x0c,0x0d,0x0f,0x10,0x12,0x13,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
0x25,0x27,0x2a,0x2c,0x2e,0x31,0x33,0x36,0x38,0x3b,0x3e,0x40,0x43,0x46,0x49,0x4c,
0x4f,0x51,0x54,0x57,0x5a,0x5d,0x60,0x63,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
}
};


#define songlength(size) (sizeof(size)/sizeof(struct notes_duration_t))

struct song_t songs[] PROGMEM = {
// { name,     delay, transpose, struct of notes, # of notes }
#ifdef ENTCHEN_PWM_MELODY_SUPPORT
  { "entchen", 10, 4, entchen_notes, songlength(entchen_notes) }, 
#endif /* ENTCHEN_PWM_MELODY_SUPPORT */
#ifdef TETRIS_PWM_MELODY_SUPPORT
  { "tetris", 30, 1, tetris_notes, songlength(tetris_notes) }, 
#endif /* TETRIS_PWM_MELODY_SUPPORT */
//  { "newsong", 40, 1, newsong_notes, songlength(newsong_notes) }
};

#define MAX_PWM_SONGS (sizeof(songs)/sizeof(struct song_t))

// Interrupt-Funktion, die den "Zeiger" hochzählt
// je nach gewünschter Frequenz wird "scale" verändert, 
// und somit die Sinuswelle schneller (hoher ton) 
// oder langsamer (tiefer Ton) abgelaufen

ISR(_PWM_MELODY_COMP){
	_PWM_MELODY_OCR=pgm_read_byte(&sinewave[pwm_melody_tone][(pwm_melody_i>>8)]);
   	pwm_melody_i += pwm_melody_scale;
}

void
pwm_melody_init(uint8_t songnr)  // Play it once, Sam!
{
	struct song_t song;
	struct notes_duration_t notes;
	if (songnr >=MAX_PWM_SONGS) // causes error if no songs activated
	  songnr=0;
	memcpy_P(&song, &songs[songnr], sizeof(struct song_t));

    PWMDEBUG("melody: title: '%s', delay: %i, size: %i: transpose: x%i, nr of songs: %i\n", song.title, song.delay, song.size, song.transpose, MAX_PWM_SONGS);
// see example at http://www.infolexikon.de/blog/atmega-music/

	// Anfangswert der PWM
	_PWM_MELODY_OCR=0x80;
	
    //DDR_CONFIG_OUT(PWMSOUND); //	allways PD7 ??? fix me!
	DDRD |= (1<<7);

	//Output compare OCxA 8 bit non inverted PWM
	// Timer Counter Control Register!
	// Bit:   7	  6	 5	4     3     2     1     0
	// Bed: COMxA1 COMxA0 COMxB1 COMxB0 FOCxA FOCxB WGMx1 WGMx0
	// Hier:  1       0      0      1     0     0     0     1
	_PWM_MELODY_TRCCRA |= (1<<_PWM_MELODY_COM1|1<<_PWM_MELODY_COM0|1<<_PWM_MELODY_WGM0); // 0x91
	
	// Timer ohne Prescaler starten
	_PWM_MELODY_TRCCRB |= (1<<_PWM_MELODY_CS0); // 0x01;
	
	// Einschalten des Ausgangs-Vergleichs-Interrupts auf OCRxA 
	// Timer/Counter Interrupt Mask!
	// Bit:   7	6      5      4     3      2     1      0
	// Bed: OCIE2 TOIE2 TICIE1 OCIE1A OCIE1B TOIE1 ------ TOIE0	
	// Hier:  0	0      0      1     0      0     0      0
	_PWM_MELODY_TIMSK |= (1 << _PWM_MELODY_OCIE); // 0x10
	//enable global interrupts
	sei();

	// durch das Noten-Array laufen und nacheinander
	// die Töne in jeweiliger Länge abspielen
	// da "scale" global definiert ist, kann es einfach
	// hier geändert werden!
	for(int y=0; y < song.size; y++){
		memcpy_P(&notes, &song.notes[y], sizeof(struct notes_duration_t));
		pwm_melody_scale = song.transpose * notes.note;

		uint16_t delay = notes.duration * 10 * song.delay / 8 ;
    	PWMDEBUG("%3i. note: %4i, dur: %3i, i: %5i, scale: %5i, delay: %5i\n", y, notes.note, notes.duration, pwm_melody_i, pwm_melody_scale, delay);

		while(delay--)
			_delay_ms(1);
		// Interrupt kurz ausschalten, gibt kurze Pause
		// so werden die Töne getrennt
		cli();
		_delay_ms(5);
		pwm_melody_i=0;
		sei();
	}
}

int16_t
parse_cmd_pwm_melody_play(char *cmd, char *output, uint16_t len)
{
  uint8_t song = atoi(cmd);
  pwm_melody_init(song);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[Sound]]/Melody support)
  ecmd_feature(pwm_melody_play, "pwm melody", [NUMBER], Play melody)
*/
