/*
 * Copyright(C) 2010 Mirko Taschenberger <mirkiway at gmx.de>
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

#ifndef ADC_ADS7822_H
#define ADC_ADS7822_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <config.h>


#if ADS_CLOCK > 2000000
    #error "ADS7822 clock frequency too high"
#endif

#define ADS_DELAY_US (1000000 / (ADS_CLOCK * 2))

uint16_t get_ads();

#endif /* ADC_ADS7822_H */
