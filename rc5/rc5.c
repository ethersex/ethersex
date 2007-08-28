/* vim:fdm=marker ts=4 et ai
 * {{{
 *         simple rc5 implementation
 *
 *    for additional information please
 *    see http://lochraster.org/etherrape
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <avr/io.h>
#include <util/delay.h>

#include "../config.h"
#include "rc5.h"

#define noinline __attribute__((noinline))

/* module local prototypes */
void noinline rc5_send_one(void);
void noinline rc5_send_zero(void);

void rc5_init(void)
{

    /* configure send pin as output, set low */
    RC5_SEND_DDR |= _BV(RC5_SEND_PINNUM);
    RC5_SEND_PORT &= ~_BV(RC5_SEND_PINNUM);

}


void rc5_send(uint8_t addr, uint8_t cmd)
{
    static uint8_t toggle = 0;

    /* send two one sync bits */
    rc5_send_one();
    rc5_send_one();

    /* send toggle bit */
    if (toggle)
        rc5_send_one();
    else
        rc5_send_zero();

    toggle = !toggle;

    for (int8_t i = 4; i >= 0; i--) {
        if (addr & _BV(i))
            rc5_send_one();
        else
            rc5_send_zero();
    }

    for (int8_t i = 5; i >= 0; i--) {
        if (cmd & _BV(i))
            rc5_send_one();
        else
            rc5_send_zero();
    }

}

void rc5_send_one(void)
{
    RC5_SEND_PORT &= ~_BV(RC5_SEND_PINNUM);
    _delay_loop_2(RC5_PULSE);
    RC5_SEND_PORT |= _BV(RC5_SEND_PINNUM);
}

void rc5_send_zero(void)
{
    RC5_SEND_PORT |= _BV(RC5_SEND_PINNUM);
    _delay_loop_2(RC5_PULSE);
    RC5_SEND_PORT &= ~_BV(RC5_SEND_PINNUM);
}
