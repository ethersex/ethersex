/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef HAVE_HBRIDGE_H
#define HAVE_HBRIDGE_H

#define HBRIDGE_PWM_STOP 0xFF

void
init_hbridge();

void
hbridge_pwm(uint8_t selection, uint8_t speed);

void
hbridge(uint8_t selection, uint8_t action);

void
dual_hbridge(uint8_t action);

enum {
	HBRIDGE_1_SELECT,
	HBRIDGE_2_SELECT,
// *** ENABLE HBRIDGE
	HBRIDGE_1_ENABLE,
	HBRIDGE_2_ENABLE,
// *** SINGLE ENGINE MOVES
	HBRIDGE_ACTION_FREE,
	HBRIDGE_ACTION_BRAKE,
	HBRIDGE_ACTION_RIGHT,
	HBRIDGE_ACTION_LEFT,
// *** ROBOT (2 engines) MOVES
	DUAL_HBRIDGE_ACTION_FREE,
	DUAL_HBRIDGE_ACTION_BRAKE,
	DUAL_HBRIDGE_ACTION_FORWARD,
	DUAL_HBRIDGE_ACTION_BACKWARD,
	DUAL_HBRIDGE_ACTION_RIGHT,
	DUAL_HBRIDGE_ACTION_LEFT,
	DUAL_HBRIDGE_ACTION_RIGHT_ONLY,
	DUAL_HBRIDGE_ACTION_LEFT_ONLY
};


#include "config.h"
#ifdef DEBUG_HBRIDGE
# include "core/debug.h"
# define HBRIDGEDEBUG(a...)  debug_printf("h-bridge: " a)
#else
# define HBRIDGEDEBUG(a...)
#endif

#endif  /* HAVE_HBRIDGE_H */
