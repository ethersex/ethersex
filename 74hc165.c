/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 *          enc28j60 api
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


#include "74hc165.h"

#ifdef USE_74HC165

void hc165_init(void)
/* {{{ */ {

    HC165_DDR |= _BV(HC165_LOAD) | _BV(HC165_CLOCK);
    HC165_DDR &= ~_BV(HC165_DATA);
    HC165_PORT |= _BV(HC165_LOAD) | _BV(HC165_CLOCK) | _BV(HC165_DATA);

} /* }}} */

uint8_t hc165_read_byte(void)
/* {{{ */ {

    uint8_t data = 0;

    /* load */
    HC165_PORT &= ~_BV(HC165_LOAD);
    HC165_DELAY();
    HC165_PORT |=  _BV(HC165_LOAD);
    HC165_DELAY();

    /* clock out */
    for (uint8_t i = 0; i < 8; i++) {

        /* clock down, up */
        HC165_PORT &= ~_BV(HC165_CLOCK);
        HC165_DELAY();
        HC165_PORT |=  _BV(HC165_CLOCK);
        HC165_DELAY();

        /* read bit */
        data <<= 1;

        if (HC165_PIN & _BV(HC165_DATA))
            data |= 0x01;

    }

    return data;

} /* }}} */

#endif
