/*
* Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef _PWM_SERVO_H
#define _PWM_SERVO_H

#ifdef PWM_SERVO_SUPPORT

void 
pwm_servo_init();
void 
setservo(uint8_t servo, uint8_t position);

#define MAXPULSFREQ 500 // 2ms => 500HZ

#define TIMER_MAXPULS F_CPU/MAXPULSFREQ // Timer2 value for a 2ms Puls

#define MINPULS TIMER_MAXPULS/4  // min pulslength = 0.5ms
#define MAXPULS TIMER_MAXPULS // max pulslength=2ms

// port pin definitions
// you may redefine the pins to suit your application
// tale a look at the interrupt routine and enable the cases for your servo

#define PWM_SERVOS 1

#define LOW_SERVO0 PORTD&=~(1<<7)
#define HIGH_SERVO0 PORTD|=(1<<7)

#define LOW_SERVO1 
#define HIGH_SERVO1

#define LOW_SERVO2
#define HIGH_SERVO2

#define LOW_SERVO3
#define HIGH_SERVO3

#define LOW_SERVO4
#define HIGH_SERVO4

#define LOW_SERVO5
#define HIGH_SERVO5

#define LOW_SERVO6
#define HIGH_SERVO6

#define LOW_SERVO7
#define HIGH_SERVO7

#define LOW_SERVO8
#define HIGH_SERVO8

#define LOW_SERVO9
#define HIGH_SERVO9



#ifdef DEBUG_PWM_SERVO
# include "core/debug.h"
# define PWMSERVODEBUG(a...)  debug_printf("pwm servo: " a)
#else
# define PWMSERVODEBUG(a...)
#endif


#endif //PWM_SERVO_SUPPORT

#endif /* _PWM_SERVO_H */

