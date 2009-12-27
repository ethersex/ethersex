/*
* delay until a bit in SRAM goes on or off
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

#ifndef _UTIL_DELAY_BIT_H_
#define _UTIL_DELAY_BIT_H_

// one iteration is 8 clock cycles (+once the setup time)
// max iterations is 524.3 ms / F_CPU in MHz
static inline 
uint16_t _delay_bit_break_on(uint16_t count, volatile uint8_t* bit, uint8_t mask)
{
    asm volatile (
        "1:" "\n\t"
        "    ld __tmp_reg__, z" "\n\t"
        "    and __tmp_reg__, %[mask]" "\n\t"
        "    brne 2f" "\n\t"
        "\n\t"
        "    sbiw %[count],1" "\n\t"
        "    brne 1b" "\n\t"
        "2:" "\n\t"
        : [count] "+w" (count)
        : [bit] "z" (bit), [mask] "r" (mask));

    return count;
}

static inline
uint16_t _delay_bit_break_off(uint16_t count, volatile uint8_t* bit, uint8_t mask)
{
    asm volatile (
        "1:" "\n\t"
        "    ld __tmp_reg__, z" "\n\t"
        "    and __tmp_reg__, %[mask]" "\n\t"
        "    breq 2f" "\n\t"
        "\n\t"
        "    sbiw %[count],1" "\n\t"
        "    brne 1b" "\n\t"
        "2:" "\n\t"
        : [count] "+w" (count)
        : [bit] "z" (bit), [mask] "r" (mask));

    return count;
}

static inline
uint16_t delay_bit(uint16_t count, volatile uint8_t* bit, uint8_t mask, uint8_t break_on)
{
    if (break_on)
        return _delay_bit_break_on(count,bit,mask);
    else
        return _delay_bit_break_off(count,bit,mask);
}

#define _COUNT_DELAY_US(us,cycles) ((uint16_t)(((double)(F_CPU) / ((double)1000000*(double)cycles))*us+0.5))

#define COUNT_DELAY_BIT_US(us) (_COUNT_DELAY_US(us,8) == 0 ? 1 : _COUNT_DELAY_US(us,8))

#endif
