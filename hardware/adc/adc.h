/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _ADC_H
#define _ADC_H

#include <inttypes.h>

#include "config.h"

#ifdef ADC_VOLTAGE_SUPPORT

#if ADC_REF == ADC_2_56
#define ADC_REF_VOLTAGE 2560
#elif ADC_REF == ADC_1_1
#define ADC_REF_VOLTAGE 1100
#else
#define ADC_REF_VOLTAGE 5000
#endif

#define ADC_RES_RECIEP (1.0F / 1023.0F)

#define adc_get_voltage(x) adc_get_voltage_setref(ADC_REF,x)

#endif /*ADC_VOLTAGE_SUPPORT */

#define adc_get(x) adc_get_setref(ADC_REF,x)

void adc_init(void);
uint16_t adc_get_setref(uint8_t ref, uint8_t channel);

int16_t parse_cmd_adc_get(char *cmd, char *output, uint16_t len);

#ifdef ADC_VOLTAGE_SUPPORT
uint16_t adc_get_voltage_setref(uint8_t ref, uint8_t channel);
uint16_t adc_raw_to_voltage(uint16_t raw);
uint16_t adc_get_vref();
void adc_set_vref(uint16_t value);

int16_t parse_cmd_adc_vget(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_adc_vref(char *cmd, char *output, uint16_t len);

#endif /*ADC_VOLTAGE_SUPPORT */

#endif /* _ADC_H */
