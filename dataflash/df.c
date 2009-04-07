/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include <avr/interrupt.h>

#include "df.h"
#include "core/bit-macros.h"
#include "core/spi.h"

/* module local macros */
#ifdef RFM12_IP_SUPPORT
/* RFM12 uses interrupts which do SPI interaction, therefore
   we have to disable interrupts if support is enabled */
#  define cs_low()  uint8_t sreg = SREG; cli(); PIN_CLEAR(SPI_CS_DF); 
#  define cs_high() PIN_SET(SPI_CS_DF); SREG = sreg;
#else
#  define cs_low()  PIN_CLEAR(SPI_CS_DF)
#  define cs_high() PIN_SET(SPI_CS_DF)
#endif


void df_init(df_chip_t chip)
{

    /* no init needed atm */

}

void df_buf_load(df_chip_t chip, df_buf_t buffer, df_page_t page)
{

    // df_wait(chip);

    cs_low();

    /* send opcode */
    if (buffer == DF_BUF1)
        spi_send(DATAFLASH_LOAD_BUFFER1);
    else
        spi_send(DATAFLASH_LOAD_BUFFER2);

    /* send 3 address bytes: 2 don't care bits, 12 bits page address,
     * 10 don't care bits */
    page <<= 2;
    spi_send(HI8(page));
    spi_send(LO8(page));
    spi_send(0);

    cs_high();

}

void df_buf_read(df_chip_t chip, df_buf_t buffer, void* data, df_size_t offset, df_size_t len)
{

    // df_wait(chip);

    cs_low();

    /* send opcode */
    if (buffer == DF_BUF1)
        spi_send(DATAFLASH_READ_BUFFER1);
    else
        spi_send(DATAFLASH_READ_BUFFER2);

    /* send 3 address bytes: 14 don't care bits, 10 bits offset within the buffer */
    spi_send(0);
    spi_send(HI8(offset));
    spi_send(LO8(offset));

    /* for dataflash revision B use full-speed mode and send one don't care byte */
#   ifdef SUPPORT_AT45DB161B
    spi_send(0);
#   endif
    /* in current revision D don't care byte is not needed, as we use the low speed opcodes 0xD1 and 0xD3 */

    /* read memory */
    uint8_t *p = (uint8_t *)data;
    while (len-- != 0)
        *p++ = spi_send(0);

    cs_high();

}

void df_buf_write(df_chip_t chip, df_buf_t buffer, void* data, df_size_t offset, df_size_t len)
{

    // df_wait(chip);

    cs_low();

    /* send opcode */
    if (buffer == DF_BUF1)
        spi_send(DATAFLASH_WRITE_BUFFER1);
    else
        spi_send(DATAFLASH_WRITE_BUFFER2);

    /* send 3 address bytes: 14 don't care bits, 10 bits offset within the buffer */
    spi_send(0);
    spi_send(HI8(offset));
    spi_send(LO8(offset));

    /* send memory */
    uint8_t *p = (uint8_t *)data;
    while (len-- != 0)
        spi_send(*p++);

    cs_high();

}

void df_buf_save(df_chip_t chip, df_buf_t buffer, df_page_t page_address)
{

    // dataflash_wait_busy();

    cs_low();

    /* send opcode */
    if (buffer == DF_BUF1)
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

}

void df_flash_read(df_chip_t chip, df_page_t page_address, void* data, df_size_t offset, df_size_t len)
{

    // dataflash_wait_busy();

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
    uint8_t *p = (uint8_t *)data;
    while (len-- != 0)
        *p++ = spi_send(0);

    cs_high();

}

void df_erase(df_chip_t chip, df_page_t page_address)
{

    // dataflash_wait_busy();

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

}

df_status_t df_status(df_chip_t chip)
{

    cs_low();

    /* send opcode and read status afterwards */
    spi_send(DATAFLASH_READ_STATUS);
    uint8_t data = spi_send(0);

    cs_high();

    return data;

}

void df_wait(df_chip_t chip)
{

    cs_low();

    /* send opcode and read status until BUSY bit is unset */
    spi_send(DATAFLASH_READ_STATUS);
    while(!(spi_send(0) & _BV(DATAFLASH_STATUS_BUSY)));

    cs_high();

}

