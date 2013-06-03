/*
*
* Copyright (c) 2011 by warhog <warhog@gmx.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
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
#ifndef HAVE_LOME6_H
#define HAVE_LOME6_H

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "hardware/onewire/onewire.h"
#include "services/clock/clock.h"
#include "core/bit-macros.h"
#include "core/tty/tty.h"

uint8_t iLCDPage;
int16_t iTemperatureCPU;
int16_t iTemperatureSB;
uint8_t iCountdownTimer;
uint32_t iUptime;
uint16_t iPOD;

#ifdef LOME6_ONEWIRE_SUPPORT
int16_t iTemperatureAIR;
int16_t iTemperaturePSU;
int16_t iTemperatureRAM;

ow_rom_code_t romcodePSU;
ow_rom_code_t romcodeAIR;
ow_rom_code_t romcodeRAM;

int16_t lome6_get_temperature(ow_rom_code_t *rom);
#endif

#ifdef LOME6_LCD_SUPPORT
WINDOW *ttyWindow;
#endif

void lome6_startup(void);
void lome6_timer(void);
void lome6_timersec(void);


#ifdef DEBUG_LOME6
# include "core/debug.h"
# define LOME6DEBUG(a...)  debug_printf("lome6: " a)
#else
# define LOME6DEBUG(a...)
#endif


#endif  /* HAVE_LOME6_H */

