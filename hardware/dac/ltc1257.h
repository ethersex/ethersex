/*
 * Support for DAC LTC1257
 * real hardware access - header file
 * Copyright (C) 2009 Meinhard Schneider <meini@meini.org>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _LTC1257_H
#define _LTC1257_H

/* how many DACs can be adressed (not hardware but memory limitation) */
#ifdef CONF_LTC1257_NUM_DEVICES
	#define LTC1257_MAX_NUM_VALUES CONF_LTC1257_NUM_DEVICES
#else
	#define LTC1257_MAX_NUM_VALUES 4
#endif


/* Debugging */
#ifdef DEBUG_LTC1257_CORE
	#include "core/debug.h"
	#define LTC1257_CORE_DEBUG(a...) debug_printf("LTC1257: " a)
#else
	#define LTC1257_CORE_DEBUG(a...)
#endif

#ifdef DEBUG_LTC1257_ECMD
	#include "core/debug.h"
	#define LTC1257_ECMD_DEBUG(a...) debug_printf("LTC1257: " a)
#else
	#define LTC1257_ECMD_DEBUG(a...)
#endif


void ltc1257_delay_get(uint16_t *d);
void ltc1257_delay_set(uint16_t *d);
void ltc1257_init();
void ltc1257_set(uint16_t *value[], uint8_t num_values);

#endif
