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

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "core/util/delay_bit.h"

#include "dali.h"

void dali_send(uint16_t *frame)
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
        uint8_t b=*((uint8_t*)(frame)+i);
        
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

#ifdef DALI_RECEIVE_SUPPORT

/** @brief read a response frame (8bit) from a DALI slave
 *  @param *frame the data (when successfully read) will be written to the target of the pointer
 *  @return DALI_READ_TIMEOUT: timeout, DALI_READ_ERROR: illegal data received, DALI_READ_OK: success
 */
int8_t dali_read(uint8_t *frame)
{
    // no inverter at the input pin, high means high
    DDR_CONFIG_IN(DALI_IN);
    *frame=0;

    // initialize ptr and mask for our input pin
    uint8_t* pin_ptr=(uint8_t*)&PIN_CHAR(DALI_IN_PORT);
    uint8_t bitmask= 1 << DALI_IN_PIN;

    // wait for the begin of the start bit, max 9.17msec
    if (!delay_bit(COUNT_DELAY_BIT_US(9170),pin_ptr,bitmask,0))
        return DALI_READ_TIMEOUT;

    uint16_t time_remaining;

    // wait for rising edge of start bit
    time_remaining=delay_bit(COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC)*1.5,pin_ptr,bitmask,1);

    if (time_remaining > COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC) ||
        time_remaining == 0)
    {
        // we are way out of timing spec
        return DALI_READ_ERROR;
    }

    // pin is high again, second half of the start bit
    uint8_t waitfor=0;

    for (int i=0; i < 8; i++)
    {
        // wait for bit change, record timing
        time_remaining=delay_bit(COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC)*2.5,pin_ptr,bitmask,waitfor);

        if (time_remaining > COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC)*2 ||
            time_remaining == 0)
        {
            // we are way out of timing spec
            return DALI_READ_ERROR;
        }
        else if (time_remaining > COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC))
        {
            // bit-change at bit-border -> don't care
            if (waitfor)
                waitfor=0;
            else
                waitfor=1;
                
            time_remaining=delay_bit(COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC)*1.5,pin_ptr,bitmask,waitfor);
            
            if (time_remaining > COUNT_DELAY_BIT_US(DALI_HALF_BIT_USEC) ||
                time_remaining == 0)
            {
                // we are way out of timing spec
                return DALI_READ_ERROR;
            }
        }
        
        // bit-change at middle of bit time: data
        *frame|=waitfor;
        if (i < 7)
            *frame <<= 1;
        
        if (waitfor)
            waitfor=0;
        else
            waitfor=1;
    }

    return DALI_READ_OK;
}

#endif

/*
  -- Ethersex META --
  header(protocols/dali/dali.h)
*/
