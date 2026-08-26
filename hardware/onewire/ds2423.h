/*
 * Support for the counters of DS2423
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

#ifndef _DS2423_H
#define _DS2423_H

#include "onewire.h"

#define OW_FAMILY_DS2423 0x1D
#define OW_DS2423_READ_MEM_COUNTER 0xA5
#define OW_DS2423_PAGE_SIZE 0x0020
#define OW_DS2423_PAGE_COUNT 16

int8_t ow_ds2423_sensor(ow_rom_code_t *rom);
int8_t ow_ds2423_get_counter(ow_rom_code_t *rom, int8_t counter, uint32_t *countervalue);

#endif
