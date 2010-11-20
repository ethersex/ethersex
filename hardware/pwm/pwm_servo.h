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
init_servos(void);
void 
pwm_servo_enable(uint8_t enable);
void 
pwm_servo_init(void);
void 
setservo(uint8_t servo, uint8_t position);
void 
servoinc(uint8_t servo);
void 
servodec(uint8_t servo);
void 
setservodelays(uint8_t index, uint8_t value);


#define SERVO_STARTVALUE 128

#define DOUBLE_PWM_SERVOS (PWM_SERVOS*2)

#define MAXPULSFREQ 150 // 2ms => 50HZ

#define TIMER_MAXPULS (F_CPU/MAXPULSFREQ) // Timer value for a 2ms Puls
#if TIMER_MAXPULS > 0xFFFF
#error Value of MAXPULSFREQ is too low
#endif

#define MINPULS (TIMER_MAXPULS/13)  // min pulslength = 0.5ms
#define MAXPULS (TIMER_MAXPULS/3) // max pulslength=2ms

#ifdef DEBUG_PWM_SERVO
# include "core/debug.h"
# define PWMSERVODEBUG(a...)  debug_printf("pwm servo: " a)
#else
# define PWMSERVODEBUG(a...)
#endif

#ifdef PWM_SERVO_INVERT
# define SERVOSET(pin) PIN_CLEAR(pin);
# define SERVOCLEAR(pin) PIN_SET(pin);
#else
# define SERVOSET(pin) PIN_SET(pin);
# define SERVOCLEAR(pin) PIN_CLEAR(pin);
#endif

#endif //PWM_SERVO_SUPPORT

#endif /* _PWM_SERVO_H */

