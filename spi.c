/* vim:fdm=marker ts=4 et ai
 * {{{
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
#include "spi.h"

void spi_init(void)
/* {{{ */ {

    /* configure MOSI, SCK, lines as outputs */
    DDR_CONFIG_OUT(SPI_MOSI);
    DDR_CONFIG_OUT(SPI_SCK);
    DDR_CONFIG_IN(SPI_MISO);

    DDRB |= _BV(PB0) | _BV(PB1);

#ifdef ENC28J60_SUPPORT
    /* set all CS high (output) */
    DDR_CONFIG_OUT(SPI_CS_NET);
    PIN_SET(SPI_CS_NET);
#endif

    PORTB |= _BV(PB0) | _BV(PB1);

#ifdef RFM12_SUPPORT
    /* initialize spi link to rfm12 module */
    DDR_CONFIG_OUT(SPI_CS_RFM12);
    /* Enable the pullup */
    PIN_SET(SPI_CS_RFM12);
#endif

    /* enable spi, set master and clock modes (f/2) */
    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
    _SPSR0 = _BV(_SPI2X0);

} /* }}} */

void noinline spi_wait_busy(void)
/* {{{ */ {

#   ifdef SPI_TIMEOUT
    uint8_t timeout = 200;

    while (!(_SPSR0 & _BV(_SPIF0)) && timeout > 0)
        timeout--;

    if (timeout == 0)
        debug_printf("ERROR: spi timeout reached!\r\n");
#   else
    while (!(_SPSR0 & _BV(_SPIF0)));
#   endif

} /* }}} */

uint8_t noinline spi_send(uint8_t data)
/* {{{ */ {

    _SPDR0 = data;
    spi_wait_busy();

    return _SPDR0;

} /* }}} */
