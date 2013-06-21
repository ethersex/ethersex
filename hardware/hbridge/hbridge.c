/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "hbridge.h"


/*
  dual H-Bridge for full engine control such as L293NE or SN754410

                  LEFT             RIGHT
 +  o--EN1_2--+------------+   +------------+--EN3_4--o +
              |            |   |            |
              S1           S3  S5           S7
              |            |   |            |
              +-Y1-(M1)-Y2-+   +-Y4-(M2)-Y3-+
              |            |   |            |
              S2           S4  S6           S8
              |            |   |            |
 -  o---------+------------+   +------------+---------o -

 S1-S8  - internal switches of the IC
 M1-M2	- engines
 Y1-Y4	- connector to engines
 A1-A4  - connector to logic for IC
 EN1_2  - enable pin for engine M1
 EN3_4  - enable pin for engine M2

  see http://en.wikipedia.org/wiki/H-bridge for details

*/

#ifdef HBRIDGE_INVERTER_SUPPORT
/*
 with an inverter IC such as CD4093, the 2nd hbridge is 
 electrically inverted to the 1st one, 
 eg. A1 and A2 always differ

- pin enable:       enable both?
- pin motor left:   forward/backward
- pin motor right:  forward/backward
*/
#endif /* HBRIDGE_INVERTER_SUPPORT */


#ifdef HBRIDGE_PWM_SUPPORT

uint8_t enable1_pwm=HBRIDGE_PWM_STOP;
#ifdef SHARE_ENABLE_HBRIDGE_SUPPORT
  #define enable2_pwm enable1_pwm
#else
uint8_t enable2_pwm=HBRIDGE_PWM_STOP;
#endif /* SHARE_ENABLE_HBRIDGE_SUPPORT */

void
hbridge_pwm(uint8_t selection, uint8_t speed){
  if (selection==HBRIDGE_1_SELECT){
    enable1_pwm=speed;
    OCR1C=enable1_pwm;
  } else {
    enable2_pwm=speed;
    OCR1C=enable2_pwm;  // fixme
  }
}

#endif /* HBRIDGE_PWM_SUPPORT */

void
init_hbridge(){
#ifdef HBRIDGE_PWM_SUPPORT
  HBRIDGEDEBUG ("pwm init\n");

  DDR_CONFIG_OUT(HBRIDGE_1_ENABLE); // PWM OUTPUT
  OCR1C=enable1_pwm;

  TC1_COUNTER_CURRENT=0x00FF; //set the timer counter

  TCCR1A|=_BV(COM1C1)|_BV(COM1C0); // Set OCnC on compare match
  TCCR1A|=_BV(WGM10);  // PWM, Phase Correct, 8-bit

  TCCR1B|=_BV(WGM12); // waveform generation mode: CTC, 
  TCCR1B|=_BV(CS10); // clockselect: clkI/O/1 (No prescaling)

  // activate PWM outports OC1C
  TCCR1C|=1<<FOC1C;

#endif /* HBRIDGE_PWM_SUPPORT */
}

void
hbridge_disable(uint8_t selection){
#ifdef HBRIDGE_PWM_SUPPORT
  if (selection==HBRIDGE_1_SELECT){
  	OCR1C=HBRIDGE_PWM_STOP;
  } else {
  	OCR1C=HBRIDGE_PWM_STOP; // fix me
  }
#else
  if (selection==HBRIDGE_1_SELECT){
	  PIN_CLEAR(HBRIDGE_1_ENABLE);
  } else {
	  PIN_CLEAR(HBRIDGE_2_ENABLE);
  }
#endif /* HBRIDGE_PWM_SUPPORT */
}

void
hbridge_enable(uint8_t selection){
#ifdef HBRIDGE_PWM_SUPPORT
  if (selection==HBRIDGE_1_SELECT){
    OCR1C=enable1_pwm;
  } else {
    OCR1C=enable1_pwm; // fix me
  }
#else
  if (selection==HBRIDGE_1_SELECT){
    PIN_SET(HBRIDGE_1_ENABLE);
  } else {
	PIN_SET(HBRIDGE_2_ENABLE);
  }
#endif /* HBRIDGE_PWM_SUPPORT */
}

void
hbridge(uint8_t selection, uint8_t action)
{
  if (selection==HBRIDGE_1_SELECT){
#ifdef HBRIDGE_INVERTER_SUPPORT
		  PIN_CLEAR(HBRIDGE_I1);
#else
		  PIN_CLEAR(HBRIDGE_A1);
		  PIN_CLEAR(HBRIDGE_A2);
#endif /* HBRIDGE_INVERTER_SUPPORT */
		  switch (action){
			case HBRIDGE_ACTION_BRAKE: 
				hbridge_disable(selection); // PIN_CLEAR(HBRIDGE_1_ENABLE);
				break;
			case HBRIDGE_ACTION_RIGHT: 
#ifdef HBRIDGE_INVERTER_SUPPORT
			 	 PIN_CLEAR(HBRIDGE_I1);
#else
				PIN_CLEAR(HBRIDGE_A1);
				PIN_SET(HBRIDGE_A2);
#endif /* HBRIDGE_INVERTER_SUPPORT */
				hbridge_enable(selection); // PIN_SET(HBRIDGE_1_ENABLE);
				break;
			case HBRIDGE_ACTION_LEFT: 
#ifdef HBRIDGE_INVERTER_SUPPORT
			  	PIN_SET(HBRIDGE_I1);
#else
				PIN_SET(HBRIDGE_A1);
				PIN_CLEAR(HBRIDGE_A2);
#endif /* HBRIDGE_INVERTER_SUPPORT */
				hbridge_enable(selection); // PIN_SET(HBRIDGE_1_ENABLE);
				break;
		  }
#ifdef DUAL_HBRIDGE_SUPPORT
  }else{
#ifdef HBRIDGE_INVERTER_SUPPORT
	 	 PIN_CLEAR(HBRIDGE_I2);
#else
		  PIN_CLEAR(HBRIDGE_A4);
		  PIN_CLEAR(HBRIDGE_A3);
#endif /* HBRIDGE_INVERTER_SUPPORT */
		  switch (action){
			case HBRIDGE_ACTION_BRAKE: 
				hbridge_disable(selection); // PIN_CLEAR(HBRIDGE_2_ENABLE);
				break;
			case HBRIDGE_ACTION_RIGHT: 
#ifdef HBRIDGE_INVERTER_SUPPORT
			  	PIN_CLEAR(HBRIDGE_I2);
#else
				PIN_CLEAR(HBRIDGE_A4);
				PIN_SET(HBRIDGE_A3);
#endif /* HBRIDGE_INVERTER_SUPPORT */
				hbridge_enable(selection); // PIN_SET(HBRIDGE_2_ENABLE);
				break;
			case HBRIDGE_ACTION_LEFT: 
#ifdef HBRIDGE_INVERTER_SUPPORT
			  	PIN_SET(HBRIDGE_I2);
#else
				PIN_SET(HBRIDGE_A4);
				PIN_CLEAR(HBRIDGE_A3);
#endif /* HBRIDGE_INVERTER_SUPPORT */
				PIN_SET(HBRIDGE_2_ENABLE);
				hbridge_enable(selection); // PIN_SET(HBRIDGE_2_ENABLE);
				break;
		  }
#endif /* DUAL_HBRIDGE_SUPPORT */
  } 
}

#ifdef DUAL_HBRIDGE_SUPPORT
void
dual_hbridge(uint8_t action)
{
#ifdef HBRIDGE_PWM_SUPPORT
  HBRIDGEDEBUG ("dual set: %i, en: (%i, %i)\n", action, enable1_pwm, enable2_pwm);
#else
  HBRIDGEDEBUG ("dual set: %i\n", action);
#endif

  switch (action){
	case DUAL_HBRIDGE_ACTION_BRAKE:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_BRAKE);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_BRAKE);
		break; 
	case DUAL_HBRIDGE_ACTION_FORWARD:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_RIGHT);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_RIGHT);
		break; 
	case DUAL_HBRIDGE_ACTION_BACKWARD:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_LEFT);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_LEFT);
		break; 
	case DUAL_HBRIDGE_ACTION_RIGHT:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_RIGHT);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_LEFT);
		break; 
	case DUAL_HBRIDGE_ACTION_RIGHT_ONLY:
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_BRAKE);
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_RIGHT);
		break; 
	case DUAL_HBRIDGE_ACTION_LEFT:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_LEFT);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_RIGHT);
		break; 
	case DUAL_HBRIDGE_ACTION_LEFT_ONLY:
		hbridge(HBRIDGE_1_SELECT,HBRIDGE_ACTION_BRAKE);
		hbridge(HBRIDGE_2_SELECT,HBRIDGE_ACTION_RIGHT);
		break; 
	}
}
#endif /* DUAL_HBRIDGE_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/hbridge/hbridge.h)
  init(init_hbridge)
*/

