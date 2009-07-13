/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright stochri (c.hab@gmx.net) Nov.2005 (GPL+)
 * see: http://www.roboternetz.de/phpBB2/zeigebeitrag.php?t=14220&highlight=servo
 *  SERVO Controller for up to 10 Servos
 *  Processor: ATMEGA 8
 *  CLOCK: 8MHZ, no prescaler set config bits of Atmega 8 correctly !
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <avr/io.h>
#include "config.h"

#ifdef PWM_SERVO_SUPPORT

#include "pwm_servo.h"
#include <avr/interrupt.h>
#include <avr/io.h>

typedef unsigned char byte;


uint16_t Pulslength[PWM_SERVOS * 2]; // array for all delays

/************************************************************************

   SIGNAL(SIG_OVERFLOWx)
   timer interrupt, generates the high and low pulses for each servo

***************************************************************************/
SIGNAL(SIG_OVERFLOW1)
{
   static byte servoindex_half=0;
#ifdef DEBUG_PWM_SERVO
   static uint16_t debugcount=0;
#endif

   switch (servoindex_half)
   {
      case 0: HIGH_SERVO0; break;
      case 1: LOW_SERVO0; break;
#if PWM_SERVOS > 0 
      case 2: HIGH_SERVO1; break;
      case 3: LOW_SERVO1; break;
#endif
#if PWM_SERVOS > 1 
      case 4: HIGH_SERVO2; break;
      case 5: LOW_SERVO2; break;
#endif
#if PWM_SERVOS > 2 
      case 6: HIGH_SERVO3; break;
      case 7: LOW_SERVO3; break;
#endif
#if PWM_SERVOS > 3 
      case 8: HIGH_SERVO4; break;
      case 9: LOW_SERVO4; break;
#endif
#if PWM_SERVOS > 4 
      case 10: HIGH_SERVO5; break;
      case 11: LOW_SERVO5; break;
#endif
#if PWM_SERVOS > 5 
      case 12: HIGH_SERVO6; break;
      case 13: LOW_SERVO6; break;
#endif
#if PWM_SERVOS > 6 
      case 14: HIGH_SERVO7; break;
      case 15: LOW_SERVO7; break;
#endif
#if PWM_SERVOS > 7 
      case 16: HIGH_SERVO8; break;
      case 17: LOW_SERVO8; break;
#endif
#if PWM_SERVOS > 8 
      case 18: HIGH_SERVO9; break;
      case 19: LOW_SERVO9; break;
#endif
   }


#ifdef DEBUG_PWM_SERVO
   if (debugcount > 800) {
     PWMSERVODEBUG("signal: idx: %i, TCNTx: %i \n", servoindex_half, Pulslength[servoindex_half]);
     debugcount = 0;
   }
   debugcount++;
#endif
   
   TCNT1 = Pulslength[servoindex_half]; // set time for next interrupt   

   servoindex_half++; // increment timervalue index

   if (servoindex_half == PWM_SERVOS * 2 ) 
      servoindex_half = 0;   // reset index
}
/************************************************************************

   void setservo(byte index, byte value)

   Set servo position
   value: 0..255

***************************************************************************/

void setservo(byte index, byte value)
{
   uint16_t wert;

   wert=MINPULS+(MAXPULS-MINPULS)/256*value;
   
   // callculate hightime
   Pulslength[index<<1]=0-wert;
   
   // sume of low and hightime for one servo is 2ms
   Pulslength[(index<<1)+1]=0-(TIMER_MAXPULS-wert);
   
   // 10 servos give you 10*2ms=20ms total cycle time

   PWMSERVODEBUG("setservo: servo: %i, wert: %i, p0: %i, p1: %i\n", index, wert, index<<1, Pulslength[index<<1], Pulslength[(index<<1)+1]);
}

/************************************************************************

   void init_servos()
   
   initialize all servos to the start position
***************************************************************************/
void init_servos()
{
   byte n;
   for(n = 0; n< PWM_SERVOS; n++) setservo(n,128);
   PWMSERVODEBUG("init servos done\n");
}

/************************************************************************

   void init(void)
   initialize the prozessor registers
***************************************************************************/

void pwm_servo_init(void)
{
    /* initialize ports */
   DDRD |= (1<<7);

    // init timer1
   TCNT1 = 0-16000;
   TCCR1A=0;  
   TCCR1B=0x01; // ohne precaler = 400hz
//   TCCR1B|=(1<<CS11); 
//   TCCR1B|=(1<<CS12|1<<CS10); // 256 prescaler
   TIMSK1 |= _BV(TOIE2) | _BV(TOIE1);

    /* allow interrupts */
//    sei();
   init_servos();
}

#endif // PWM_SERVO_SUPPORT
