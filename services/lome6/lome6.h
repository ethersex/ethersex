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
#include "config.h"
#include "hardware/onewire/onewire.h"
#include "hardware/lcd/hd44780.h"
#include "services/clock/clock.h"
#include "core/bit-macros.h"


uint8_t iLCDPage = 0;
int16_t iTemperatureCPU = 0;
int16_t iTemperatureSB = 0;
uint8_t iCountdownTimer = 0;
uint32_t iUptime = 0;

#ifdef LOME6_ONEWIRE_SUPPORT
int16_t iTemperatureAIR = 0;
int16_t iTemperaturePSU = 0;
int16_t iTemperatureRAM = 0;

struct ow_rom_code_t romcodePSU;
struct ow_rom_code_t romcodeAIR;
struct ow_rom_code_t romcodeRAM;
#endif

#ifdef LOME6_LCD_SUPPORT
void lome6_lcdString(char *string);
void lome6_lcdClear(void);
#define lome6_lcdGoto(x, y) hd44780_goto(y, x)
void lome6_output_lcd(char *line1, char *line2);
void lome6_lcd_temperature(char *type, int temperature, int decimal);
#endif


#ifdef LOME6_ONEWIRE_SUPPORT
void lome6_init_sensorid(void);
void lome6_read_sensorid(struct ow_rom_code_t *romcode, char *sensor);
int16_t lome6_get_temperature(struct ow_rom_code_t *rom);
#endif

void lome6_startup(void);
void lome6_timer(void);
void lome6_timersec(void);


#endif  /* HAVE_LOME6_H */

