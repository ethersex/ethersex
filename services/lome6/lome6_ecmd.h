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
#ifndef HAVE_LOME6_ECMD_H
#define HAVE_LOME6_ECMD_H

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_arp.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/uip/parse.h"
#include "config.h"
#include "services/clock/clock.h"
#include "core/bit-macros.h"

extern int16_t iTemperatureCPU;
extern int16_t iTemperatureSB;
extern uint8_t iCountdownTimer;

#ifdef LOME6_ONEWIRE_SUPPORT
extern int16_t iTemperatureAIR;
extern int16_t iTemperaturePSU;
extern int16_t iTemperatureRAM;
#endif
extern uint32_t iUptime;

int16_t parse_cmd_lome6_reset(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lome6_power(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lome6_state(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lome6_set_t(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lome6_get_t(char *cmd, char *output, uint16_t len);

#endif
