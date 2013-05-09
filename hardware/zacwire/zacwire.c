/*
* Read and convert data from a ZACwire device (TSic temp-sensors)
*
* Copyright (c) 2009 by Gerd v. Egidy <gerd@egidy.de>
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
#include <avr/interrupt.h>

#include "config.h"
#include "zacwire.h"

#include "core/util/delay_bit.h"

#include "core/debug.h"

// read one byte from zacwire, including start & parity
// when called, the pin must be already low for the first part of the startbit
static int8_t zacwire_read_byte(uint8_t *byte, uint8_t *pin_ptr, uint8_t bitmask)
{
    uint16_t time_remaining = 0;
    uint16_t raw_byte = 0;
    uint8_t i = 0;
    uint8_t parity = 0;

    // pin is low during first part of the start bit
    if (!delay_bit(COUNT_DELAY_BIT_US(65),pin_ptr,bitmask,1))
        return ZACWIRE_ERR_TIMEOUT;

    // pin is high now

    // first data byte
    // read 8 data bits and 1 parity bit
    for (i = 0; i < 9; i++)
    {
        // wait for falling edge: start of data
        // can never take longer than bit window of 125us
        if(!delay_bit(COUNT_DELAY_BIT_US(125),pin_ptr,bitmask,0))
            return ZACWIRE_ERR_TIMEOUT;

        // manchester duty cycle: value encoded in timing of rising edge
        time_remaining=delay_bit(COUNT_DELAY_BIT_US(125),pin_ptr,bitmask,1);

        if (time_remaining > COUNT_DELAY_BIT_US(62.5))
        {
            // when rising edge before half of bit window: 1
            raw_byte |= 1 << (8-i);         // get the bit
        }
        else if (time_remaining > 0)
        {
            // when rising edge after half of bit window: 0
        }
        else
        {
            // timeout
            return ZACWIRE_ERR_TIMEOUT;
        }

        // pin is high now
    }

    // parity check
    for (i = 0; i < 9; i++)
        if (raw_byte & (1 << i))
            parity++;

    // even parity
    if (parity % 2)
        return ZACWIRE_ERR_PARITY;

    // remove parity bit
    *byte = raw_byte >> 1;

    return ZACWIRE_OK;
}

// get one zacwire transmission consisting of 2 bytes
// sensor must be powered up
// the IO defined by pin_ptr and bitno must already be switched to input, pullup off
// pin_ptr: regular (!) pointer (means >0x20) to the PIN register of the chosen port
int8_t zacwire_get(uint16_t *raw_temp, uint8_t *pin_ptr, uint8_t bitno)
{
    int8_t i = 0;
    uint8_t byte_buf = 0;
    uint8_t bitmask;

    // create bitmask
    bitmask = 1 << bitno;

    // wait for stabilization if the sensor was just switched on
    _delay_us(100);

    // the pin must stay high at least for 125us
    // otherwise we are in the middle of a data-period
    // and must wait for the next one
    while (delay_bit(COUNT_DELAY_BIT_US(125),pin_ptr,bitmask,0))
    {
        // wait until the data period is over
        if (i > 20)
        {
            // no zacwire transmission has more than 20 lows -> error
            return ZACWIRE_ERR_PROTOCOL;
        }
        i++;

        // wait till the pin goes high again or timeout
        delay_bit(COUNT_DELAY_BIT_US(125),pin_ptr,bitmask,1);
    }

    // wait for data period to begin
    // timeout at 110ms - zac data is transmitted at 10Hz
    for (i=0; i < 110; i++)
    {
        if (delay_bit(COUNT_DELAY_BIT_US(1000),pin_ptr,bitmask,0))
            goto zacwire_xmit_start;
    }
    return ZACWIRE_ERR_TIMEOUT;

zacwire_xmit_start:

    // first byte
    i=zacwire_read_byte(&byte_buf,pin_ptr,bitmask);
    if (i != ZACWIRE_OK)
        return i;

    *raw_temp = byte_buf;
    *raw_temp <<= 8;

    // stop bit: high for half bit window + remaining high-time from last bit)
    if(!delay_bit(COUNT_DELAY_BIT_US(250),pin_ptr,bitmask,0))
        return ZACWIRE_ERR_TIMEOUT;

    // second byte
    i=zacwire_read_byte(&byte_buf,pin_ptr,bitmask);

    *raw_temp |= byte_buf;

    return i;
}

int16_t convert_tsic306(uint16_t raw_temp)
{
    // formula from 306 datasheet: raw_temp / 2047 * 200 - 50 in 0.1 fixedpoint
    return (int16_t)((uint16_t)((((uint32_t)raw_temp * (20000000/2047))+5000)/10000)-500);
}

int16_t convert_tsic506(uint16_t raw_temp)
{
    // formula from 506 datasheet: raw_temp / 2047 * 70 - 10 in 0.01 fixedpoint
    return (int16_t)((uint16_t)((((uint32_t)raw_temp * (700000000/2047))+50000)/100000)-1000);
}

/*
  -- Ethersex META --
  header(hardware/zacwire/zacwire.h)
*/
