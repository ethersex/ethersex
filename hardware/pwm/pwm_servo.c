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

#if PWM_SERVOS < 1
#error Value PWM_SERVO set to low!
#endif
#if PWM_SERVOS > 10
#error Value PWM_SERVO set to high!
#endif

uint16_t Pulslength[DOUBLE_PWM_SERVOS]; // array for all delays
uint8_t pos[DOUBLE_PWM_SERVOS]; // array for all positions
#ifdef PWM_SERVO_DEFAULT_ENABLED
uint8_t pwm_servo_enabled = 1;
#else
uint8_t pwm_servo_enabled = 0;
#endif /* PWM_SERVO_DEFAULT_ENABLED */

/************************************************************************

   ISR(TIMERx_OVF_vect)
   timer interrupt, generates the high and low pulses for each servo

***************************************************************************/
ISR(TC1_VECTOR_OVERFLOW)
{
 static uint8_t servoindex_half=0;
 if (pwm_servo_enabled==1) {
   switch (servoindex_half)
   {
    #ifndef HAVE_SERVO0
      #error "SERVO 0 PIN not defined"
    #endif
      case 0: SERVOSET(SERVO0); break;
      case 1: SERVOCLEAR(SERVO0); break;
#if PWM_SERVOS > 1
    #ifndef HAVE_SERVO1
      #error "SERVO 1 PIN not defined"
    #endif
      case 2: SERVOSET(SERVO1); break;
      case 3: SERVOCLEAR(SERVO1); break;
#endif
#if PWM_SERVOS > 2
    #ifndef HAVE_SERVO2
      #error "SERVO 2 PIN not defined"
    #endif
      case 4: SERVOSET(SERVO2); break;
      case 5: SERVOCLEAR(SERVO2); break;
#endif
#if PWM_SERVOS > 3
    #ifndef HAVE_SERVO3
      #error "SERVO 3 PIN not defined"
    #endif
      case 6: SERVOSET(SERVO3); break;
      case 7: SERVOCLEAR(SERVO3); break;
#endif
#if PWM_SERVOS > 4
    #ifndef HAVE_SERVO4
      #error "SERVO 4 PIN not defined"
    #endif
      case 8: SERVOSET(SERVO4); break;
      case 9: SERVOCLEAR(SERVO4); break;
#endif
#if PWM_SERVOS > 5
    #ifndef HAVE_SERVO5
      #error "SERVO 5 PIN not defined"
    #endif
      case 10: SERVOSET(SERVO5); break;
      case 11: SERVOCLEAR(SERVO5); break;
#endif
#if PWM_SERVOS > 6
    #ifndef HAVE_SERVO6
      #error "SERVO 6 PIN not defined"
    #endif
      case 12: SERVOSET(SERVO6); break;
      case 13: SERVOCLEAR(SERVO6); break;
#endif
#if PWM_SERVOS > 7
    #ifndef HAVE_SERVO7
      #error "SERVO 7 PIN not defined"
    #endif
      case 14: SERVOSET(SERVO7); break;
      case 15: SERVOCLEAR(SERVO7); break;
#endif
#if PWM_SERVOS > 8
    #ifndef HAVE_SERVO8
      #error "SERVO 8 PIN not defined"
    #endif
      case 16: SERVOSET(SERVO8); break;
      case 17: SERVOCLEAR(SERVO89; break;
#endif
#if PWM_SERVOS > 9
    #ifndef HAVE_SERVO9
      #error "SERVO 9 PIN not defined"
    #endif
      case 18: SERVOSET(SERVO9); break;
      case 19: SERVOCLEAR(SERVO9); break;
#endif
   }

   TC1_COUNTER_CURRENT = (uint16_t)(0-Pulslength[servoindex_half]); // set time for next interrupt   

   servoindex_half++; // increment timervalue index

   if (servoindex_half == DOUBLE_PWM_SERVOS) 
      servoindex_half = 0;   // reset index
  }
}
/************************************************************************

   void setservo(byte index, byte value)

   Set servo position
   value: 0..255

***************************************************************************/

void setservodelays(uint8_t index, uint8_t value)
{
   uint16_t wert;
   uint8_t puls=(uint8_t)(index<<1);

   wert=(uint16_t)(MINPULS+(MAXPULS-MINPULS)/256*value);
   
   // calculate hightime
   Pulslength[puls]=wert;
   
   // sume of low and hightime for one servo is 2ms
   Pulslength[puls+1]=(uint16_t)(TIMER_MAXPULS-wert);
   
   // 10 servos give you 10*2ms=20ms total cycle time

   PWMSERVODEBUG("set servo-nr: %u, wert: %u(%u), puls: %u=%u/%u\n", index, wert, value, puls, Pulslength[puls], Pulslength[puls+1]);
}

void setservo(uint8_t index, uint8_t value){
  pos[index]=value;  
  setservodelays(index, pos[index]);
}
void servoinc(uint8_t index){
  pos[index]++;
  setservodelays(index, pos[index]);
}
void servodec(uint8_t index){
  pos[index]--;
  setservodelays(index, pos[index]);
}
/************************************************************************

   void init_servos()
   
   initialize all servos to the start position
***************************************************************************/
void init_servos(void)
{
   uint8_t n;
   for(n = 0; n < PWM_SERVOS; n++) 
	setservo(n,SERVO_STARTVALUE);
   PWMSERVODEBUG("servo value %i, TIMER_MAXPULS: %u, MINPULS: %u, MAXPULS: %u\n",SERVO_STARTVALUE, TIMER_MAXPULS, MINPULS, MAXPULS);
}

/************************************************************************

   void init(void)
   initialize the prozessor registers
***************************************************************************/

void pwm_servo_init(void)
{
    // init timer1
   TC1_COUNTER_CURRENT = (uint16_t) 0 - 16000;
   TCCR1A = 0x00;  
   TCCR1B = 0x00; // init
   TCCR1B |= (1<<CS10);   // prescale no ->50hz
   TIMSK1 |= _BV(TOIE1);

   init_servos();
}

void 
pwm_servo_enable(uint8_t enable){
	pwm_servo_enabled = enable;
}

/*
  -- Ethersex META --
  header(hardware/pwm/pwm_servo.h)
  init(pwm_servo_init)
*/

#endif // PWM_SERVO_SUPPORT
