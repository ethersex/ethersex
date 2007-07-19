/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 *          fs20 sender implementation
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
#include <util/parity.h>
#include "fs20.h"
#include "bit-macros.h"

/* module-local prototypes */
static void fs20_send_zero(void);
static void fs20_send_one(void);
static void fs20_send_sync(void);
static inline void fs20_send_bit(uint8_t bit);
static inline void fs20_send_byte(uint8_t byte);

void fs20_send_zero(void)
/* {{{ */ {

    FS20_PORT |= _BV(FS20_PINNUM);
    _delay_loop_2(FS20_DELAY_ZERO);
    FS20_PORT &= ~_BV(FS20_PINNUM);
    _delay_loop_2(FS20_DELAY_ZERO);

} /* }}} */

void fs20_send_one(void)
/* {{{ */ {

    FS20_PORT |= _BV(FS20_PINNUM);
    _delay_loop_2(FS20_DELAY_ONE);
    FS20_PORT &= ~_BV(FS20_PINNUM);
    _delay_loop_2(FS20_DELAY_ONE);

} /* }}} */

void fs20_send_sync(void)
/* {{{ */ {

    for (uint8_t i = 0; i < 12; i++)
        fs20_send_zero();

    fs20_send_one();

} /* }}} */

void fs20_send_bit(uint8_t bit)
/* {{{ */ {

    if (bit > 0)
        fs20_send_one();
    else
        fs20_send_zero();

} /* }}} */

void fs20_send_byte(uint8_t byte)
/* {{{ */ {

    uint8_t i = 7;

    do {
        fs20_send_bit(byte & _BV(i));
    } while (i-- > 0);

    fs20_send_bit(parity_even_bit(byte));

} /* }}} */

void fs20_send(uint16_t housecode, uint8_t address, uint8_t command)
/* {{{ */ {

    for (uint8_t i = 0; i < 3; i++) {
        fs20_send_sync();

        uint8_t sum = 6; /* magic constant 6 from fs20 protocol definition... */

        fs20_send_byte(HI8(housecode));
        sum += HI8(housecode);
        fs20_send_byte(LO8(housecode));
        sum += LO8(housecode);
        fs20_send_byte(address);
        sum += address;
        fs20_send_byte(command);
        sum += command;
        fs20_send_byte(sum);

        fs20_send_zero();

        _delay_loop_2(FS20_DELAY_CMD);
    }

} /* }}} */

void fs20_init(void)
/* {{{ */ {

    FS20_DDR |= _BV(FS20_PINNUM);
    FS20_PORT &= ~_BV(FS20_PINNUM);

} /* }}} */
