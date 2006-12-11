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

#include "dataflash.h"
#include "spi.h"

#ifdef DEBUG
#include "uart.h"
#endif

/* module local macros */
#define cs_low() SPI_PORT &= ~_BV(SPI_CS_DF)
#define cs_high() SPI_PORT |= _BV(SPI_CS_DF)

uint8_t dataflash_read_status(void)
/* {{{ */ {

    cs_low();

    /* send opcode and read status afterwards */
    spi_send(DATAFLASH_READ_STATUS);
    uint8_t data = spi_send(0);

    cs_high();

    return data;

} /* }}} */

void dataflash_wait_busy(void)
/* {{{ */ {

    cs_low();

    /* send opcode and read status until BUSY bit is unset */
    spi_send(DATAFLASH_READ_STATUS);
    while(!(spi_send(0) & _BV(DATAFLASH_STATUS_BUSY)));

    cs_high();

} /* }}} */

void dataflash_read_flash(uint16_t page_address, uint16_t offset, uint8_t *data, uint16_t len)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    spi_send(DATAFLASH_MAIN_MEMORY_PAGE_READ);

    /* send 3 address bytes: 2 don't care bits, 12 bits page address,
     * 10 bits byte offset */
    page_address <<= 2;
    spi_send(HI8(page_address));
    spi_send(LO8(page_address) | HI8(offset));
    spi_send(LO8(offset));

    /* send 4 don't care bytes */
    for (uint8_t i = 0; i < 4; i++)
        spi_send(0);

    /* read memory */
    while (len-- != 0)
        *data++ = spi_send(0);

    cs_high();

} /* }}} */

void dataflash_load_buffer(uint8_t buffer, uint16_t page_address)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    if (buffer == 1)
        spi_send(DATAFLASH_LOAD_BUFFER1);
    else
        spi_send(DATAFLASH_LOAD_BUFFER2);

    /* send 3 address bytes: 2 don't care bits, 12 bits page address,
     * 10 don't care bits */
    page_address <<= 2;
    spi_send(HI8(page_address));
    spi_send(LO8(page_address));
    spi_send(0);

    cs_high();

} /* }}} */

void dataflash_read_buffer(uint8_t buffer, uint16_t offset, uint8_t *data, uint16_t len)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    if (buffer == 1)
        spi_send(DATAFLASH_READ_BUFFER1);
    else
        spi_send(DATAFLASH_READ_BUFFER2);

    /* send 3 address bytes: 14 don't care bits, 10 bits offset within the buffer */
    spi_send(0);
    spi_send(HI8(offset));
    spi_send(LO8(offset));

    /* send one don't care byte */
    spi_send(0);

    /* read memory */
    while (len-- != 0)
        *data++ = spi_send(0);

    cs_high();

} /* }}} */

void dataflash_write_buffer(uint8_t buffer, uint16_t offset, uint8_t *data, uint16_t len)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    if (buffer == 1)
        spi_send(DATAFLASH_WRITE_BUFFER1);
    else
        spi_send(DATAFLASH_WRITE_BUFFER2);

    /* send 3 address bytes: 14 don't care bits, 10 bits offset within the buffer */
    spi_send(0);
    spi_send(HI8(offset));
    spi_send(LO8(offset));

    /* send memory */
    while (len-- != 0)
        spi_send(*data++);

    cs_high();

} /* }}} */

void dataflash_save_buffer(uint8_t buffer, uint16_t page_address)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    if (buffer == 1)
        spi_send(DATAFLASH_SAVE_BUFFER1);
    else
        spi_send(DATAFLASH_SAVE_BUFFER2);

    /* send 3 address bytes: 2 don't care bits, 12 bits page address,
     * 10 don't care bits */
    page_address <<= 2;
    spi_send(HI8(page_address));
    spi_send(LO8(page_address));
    spi_send(0);

    cs_high();

} /* }}} */

void dataflash_erase_page(uint16_t page_address)
/* {{{ */ {

    dataflash_wait_busy();

    cs_low();

    /* send opcode */
    spi_send(DATAFLASH_PAGE_ERASE);

    /* send 3 address bytes: 2 don't care bits, 12 bits page address, 10
     * don't care bits */
    page_address <<= 2;
    spi_send(HI8(page_address));
    spi_send(LO8(page_address));
    spi_send(0);

    cs_high();

} /* }}} */
