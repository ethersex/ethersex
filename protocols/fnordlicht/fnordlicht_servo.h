/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef FNORDLICHT_SERVO_H
#define FNORDLICHT_SERVO_H 1

// protocol stuff
// state enable of pwm
#define SERVO_ENABLE				0xA0 
// state disable of pwm
#define SERVO_DISABLE				0xA1 
// set single servo to specific position (with step and speed)
#define SERVO_SET_SINGLE			0xA2 
// set all servos at once (max 10)
#define SERVO_SET_ALL				0xA3 
// set min position of servo
#define SERVO_SET_MIN_ALL			0xA4 
// set max position of servo
#define SERVO_SET_MAX_ALL			0xA5 
// set init position
#define SERVO_SET_INIT				0xA6 
// save pos and state to eeprom for init and default
#define SERVO_SAVE	 				0xA7 


void
fnordlicht_servo_init(void);

void parseBuffer();

#endif	/* FNORDLICHT_SERVO_H */
