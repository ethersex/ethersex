/* Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "hardware/adc/adc.h"

#ifndef ADC_KTY81_H
#define ADC_KTY81_H

#define get_kty(x) adc_get(x)

int8_t
kty_calibrate(uint16_t sensorwert);

int16_t
temperatur(uint16_t sensorwert);

void 
temp2text(char *textbuf, int16_t temperatur);

#endif /* ADC_KTY81_H */
