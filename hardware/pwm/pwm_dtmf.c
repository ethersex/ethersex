/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2000, Atmel Corporation All rights reserved.
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
 */

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "pwm_common.h"
#include "pwm_dtmf.h"
#include "protocols/ecmd/ecmd-base.h"

//************************** SIN TABLE *************************************
// Samples table : one period sampled on 128 samples and
// quantized on 7 bit
//**************************************************************************
const PROGMEM unsigned char auc_SinParam [128] = {
64,67, 70,73, 76,79, 82,85, 88,91, 94,96, 99,102, 104,106,
109,111, 113,115, 117,118, 120,121, 123,124, 125,126, 126,127, 127,127,
127,127, 127,127, 126,126, 125,124, 123,121, 120,118, 117,115, 113,111,
109,106, 104,102, 99,96, 94,91, 88,85, 82,79, 76,73, 70,67,
64,60, 57,54, 51,48, 45,42, 39,36, 33,31, 28,25, 23,21,
18,16, 14,12, 10,9, 7,6, 4,3, 2,1, 1,0, 0,0,
0,0, 0,0, 1,1, 2,3, 4,6, 7,9, 10,12, 14,16,
18,21, 23,25, 28,31, 33,36, 39,42, 45,48, 51,54, 57,60};

struct dtmf_t dtmf_tab[] PROGMEM = {
		{'1', F1209, F697},
		{'2', F1336, F697},
		{'3', F1477, F697},
		{'A', F1633, F697},
		{'4', F1209, F770},
		{'5', F1336, F770},
		{'6', F1477, F770},
		{'B', F1633, F770},
		{'7', F1209, F852},
		{'8', F1336, F852},
		{'9', F1477, F852},
		{'C', F1633, F852},
		{'*', F1209, F941},
		{'0', F1336, F941},
		{'#', F1477, F941},
		{'D', F1633, F941}
};

//**************************  global variables  ****************************
uint8_t x_SWa = 0x00;               // step width of high frequency
uint8_t x_SWb = 0x00;               // step width of low frequency
unsigned int  i_CurSinValA = 0;           // position freq. A in LUT (extended format)
unsigned int  i_CurSinValB = 0;           // position freq. B in LUT (extended format)
unsigned int  i_TmpSinValA;               // position freq. A in LUT (actual position)
unsigned int  i_TmpSinValB;               // position freq. B in LUT (actual position)

//**************************************************************************
// Timer overflow interrupt service routine
//**************************************************************************
ISR(_PWM_MELODY_COMP){
  // move Pointer about step width aheaed
  i_CurSinValA += x_SWa;       
  i_CurSinValB += x_SWb;
  // normalize Temp-Pointer
  i_TmpSinValA  =  (char)(((i_CurSinValA+4) >> 3)&(0x007F)); 
  i_TmpSinValB  =  (char)(((i_CurSinValB+4) >> 3)&(0x007F));
  // calculate PWM value: high frequency value + 3/4 low frequency value

  _PWM_MELODY_OCR = (pgm_read_byte(&auc_SinParam[i_TmpSinValA]) + (pgm_read_byte(&auc_SinParam[i_TmpSinValB])-(pgm_read_byte(&auc_SinParam[i_TmpSinValB])>>2)));
}

//**************************************************************************
// Initialization
//**************************************************************************
void pwm_dtmf_init (void)
{
	_PWM_MELODY_TIMSK = (1 <<_PWM_MELODY_OCIE);
	_PWM_MELODY_TRCCRA = (1<<_PWM_MELODY_COM1|1<<_PWM_MELODY_COM0|1<<_PWM_MELODY_WGM0); 
	_PWM_MELODY_TRCCRB = (1<<_PWM_MELODY_CS0);
	DDRD |= (1<<7);
}

//**************************************************************************
// tone to generate, extract mixing high frequency
// (column) and low frequency (row), and then
// fix x_SWa and x_SWb
//**************************************************************************

void 
dtmf(char input) {
    x_SWa = 0;
    x_SWb = 0;
    for (uint8_t i=0;i<16;i++){
		if (pgm_read_byte(&dtmf_tab[i].character)==input){
      		x_SWa = pgm_read_byte(&dtmf_tab[i].high);
      		x_SWb = pgm_read_byte(&dtmf_tab[i].low);
			_delay_ms(PWM_DTMF_SIGNAL);
			break;
		}
     }
	PWMDEBUG("code: %c: %i, %i\n", input, x_SWa, x_SWb);
    x_SWa = 0;
    x_SWb = 0;
	_delay_ms(PWM_DTMF_SIGNAL_BREAK);
}

int16_t
parse_cmd_pwm_dtmf(char *cmd, char *output, uint16_t len)
{
	uint8_t ptr=0;
	while (cmd[ptr] != 0) {
	  dtmf(cmd[ptr++]);
    }
    
    return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(hardware/pwm/pwm_dtmf.h)
  init(pwm_dtmf_init)
  ecmd_feature(pwm_dtmf, "dtmf ", CHARS, send CHARS as DTMF)
*/
