/*
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

#include "config.h"
#include "core/debug.h"

#include "adc.h"

uint16_t
hr20_batt_get (void)
{
//    #ifndef CONFIG_ADC_AVCC
//    #error ADC REF must be AVcc!
//    #endif

    uint32_t centivolt = 112640 / adc_get(0x1e);

    debug_printf ("get batt: %d cV\n", centivolt);
//    DEBUG("get batt: %d cV", (uint16_t) centivolt);

    return (uint16_t) centivolt;

}


