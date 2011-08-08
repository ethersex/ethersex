/*
* DALI light control
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
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

#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"

#include "dali.h"

void dali_send(uint16_t *data)
{
    DDR_CONFIG_OUT(DALI_OUT);

    // we expect an inverter in the output stage:
    // atmega output normally low, dali bus normally high
    
    // DALI uses Manchester encoding:
    // rising edge at the half of the bit time means 1

    // with the inverter:
    // rising edge (SET) is 0
    // falling edge (CLEAR) is 1
    
    // start bit '1'
    PIN_SET(DALI_OUT);
    DALI_HALF_BIT_WAIT;
    PIN_CLEAR(DALI_OUT);
    DALI_HALF_BIT_WAIT;

    // transmit the bytes
    for (uint8_t i=0; i<2; i++)
    {
        uint8_t b=*((uint8_t*)(data)+i);
        
        // most significant bit first
        for (uint8_t j = 0; j < 8; j++, b <<= 1)
        {
            // we are between two bits
            
            if (b & 0x80)
            {
                // bit is set
                PIN_SET(DALI_OUT);
                DALI_HALF_BIT_WAIT;
                PIN_CLEAR(DALI_OUT);
                DALI_HALF_BIT_WAIT;
            }
            else
            {
                // bit is clear
                PIN_CLEAR(DALI_OUT);
                DALI_HALF_BIT_WAIT;
                PIN_SET(DALI_OUT);
                DALI_HALF_BIT_WAIT;
            }
        }
    }
    
    // 2 stop bits always high
    PIN_CLEAR(DALI_OUT);
    DALI_HALF_BIT_WAIT;
    DALI_HALF_BIT_WAIT;
    DALI_HALF_BIT_WAIT;
    DALI_HALF_BIT_WAIT;
}


/*
  -- Ethersex META --
  header(protocols/dali/dali.h)
*/
