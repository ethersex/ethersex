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

#include <avr/io.h>
#include <util/delay.h>

#include "config.h"
#include "common.h"
#include "enc28j60.h"
#include "network.h"
#include "uart.h"
#include "uip_arp.h"

/* global variables */
uint8_t enc28j60_current_bank = 0;
int16_t enc28j60_next_packet_pointer;

void noinline wait_spi_busy(void)
/* {{{ */ {

#   ifdef SPI_TIMEOUT
    uint8_t timeout = 200;

    while (!(_SPSR0 & _BV(_SPIF0)) && timeout > 0)
        timeout--;

    if (timeout == 0)
        uart_puts_P("ERROR: isp timeout reached!\r\n");
#   else
    while (!(_SPSR0 & _BV(_SPIF0)));
#   endif

} /* }}} */

uint8_t read_control_register(uint8_t address)
/* {{{ */ {

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < 0x1B &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode and address */
    _SPDR0 = (CMD_RCR | (address & REGISTER_ADDRESS_MASK) );
    wait_spi_busy();

    /* read data */
    _SPDR0 = 0x00;
    wait_spi_busy();

    /* if this is a register in MAC or MII (when MSB is set),
     * read a dummy byte first */
    if (address & _BV(7)) {
        _SPDR0 = 0x00;
        wait_spi_busy();
    }

    /* release device */
    cs_high();

    return _SPDR0;

} /* }}} */

uint8_t read_buffer_memory(void)
/* {{{ */ {

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_RBM);
    wait_spi_busy();

    /* read data */
    _SPDR0 = 0x00;
    wait_spi_busy();

    /* release device */
    cs_high();

    return _SPDR0;

} /* }}} */

void write_control_register(uint8_t address, uint8_t data)
/* {{{ */ {

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < 0x1B &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_WCR | (address & REGISTER_ADDRESS_MASK) );
    wait_spi_busy();

    /* send data */
    _SPDR0 = data;
    wait_spi_busy();

    /* release device */
    cs_high();

} /* }}} */

void write_buffer_memory(uint8_t data)
/* {{{ */ {

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_WBM);
    wait_spi_busy();

    /* send data */
    _SPDR0 = data;
    wait_spi_busy();

    /* release device */
    cs_high();

} /* }}} */

void bit_field_set(uint8_t address, uint8_t mask)
/* {{{ */ {

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < 0x1B &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_BFS | (address & REGISTER_ADDRESS_MASK) );
    wait_spi_busy();

    /* send data */
    _SPDR0 = mask;
    wait_spi_busy();

    /* release device */
    cs_high();

} /* }}} */

void bit_field_clear(uint8_t address, uint8_t mask)
/* {{{ */ {

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < 0x1B &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_BFC | (address & REGISTER_ADDRESS_MASK) );
    wait_spi_busy();

    /* send data */
    _SPDR0 = mask;
    wait_spi_busy();

    /* release device */
    cs_high();

} /* }}} */

void noinline set_read_buffer_pointer(uint16_t address)
/* {{{ */ {

    write_control_register(REG_ERDPTL, LOW(address));
    write_control_register(REG_ERDPTH, HIGH(address));

} /* }}} */

uint16_t noinline get_read_buffer_pointer(void)
/* {{{ */ {

    return (read_control_register(REG_ERDPTL) | (read_control_register(REG_ERDPTH) << 8));

} /* }}} */

void noinline set_write_buffer_pointer(uint16_t address)
/* {{{ */ {

    write_control_register(REG_EWRPTL, LOW(address));
    write_control_register(REG_EWRPTH, HIGH(address));

} /* }}} */

uint16_t noinline get_write_buffer_pointer(void)
/* {{{ */ {

    return (read_control_register(REG_EWRPTL) | (read_control_register(REG_EWRPTH) << 8));

} /* }}} */

uint16_t read_phy(uint8_t address)
/* {{{ */ {

    write_control_register(REG_MIREGADR, address);
    bit_field_set(REG_MICMD, _BV(MIIRD));

    /* wait for this operation to complete */
    while(read_control_register(REG_MISTAT) & _BV(BUSY));

    /* stop reading */
    bit_field_clear(REG_MICMD, _BV(MIIRD));

    return (read_control_register(REG_MIRDL) | (read_control_register(REG_MIRDH) << 8));

} /* }}} */

void write_phy(uint8_t address, uint16_t data)
/* {{{ */ {

    /* set address */
    write_control_register(REG_MIREGADR, address);

    /* set data */
    write_control_register(REG_MIWRL, LOW(data));
    write_control_register(REG_MIWRH, HIGH(data));

    /* start writing and wait */
    while(read_control_register(REG_MISTAT) & _BV(BUSY));

} /* }}} */

void reset_controller(void)
/* {{{ */ {

    /* aquire device */
    cs_low();

    /* send opcode */
    _SPDR0 = (CMD_RESET);
    wait_spi_busy();

    /* wait until the controller is ready */
    while (!(read_control_register(REG_ESTAT) & _BV(CLKRDY)));

    /* release device */
    cs_high();

} /* }}} */

void reset_rx(void)
/* {{{ */ {

    /* reset rx logic */
    bit_field_set(REG_ECON1, _BV(ECON1_RXRST));
    _delay_loop_2(40000);
    bit_field_clear(REG_ECON1, _BV(ECON1_RXRST));

    /* re-enable rx */
    bit_field_set(REG_ECON1, _BV(ECON1_RXEN));

} /* }}} */

void init_enc28j60(void)
/* {{{ */ {

    reset_controller();

    /* set receive buffer to span from 0 to 4kb */
    write_control_register(REG_ERXSTL, LOW(RXBUFFER_START));
    write_control_register(REG_ERXSTH, HIGH(RXBUFFER_START));
    write_control_register(REG_ERXNDL, LOW(RXBUFFER_END));
    write_control_register(REG_ERXNDH, HIGH(RXBUFFER_END));

    /* set transmit buffer start at 4kb */
    write_control_register(REG_ETXSTL, LOW(TXBUFFER_START));
    write_control_register(REG_ETXSTH, HIGH(TXBUFFER_START));

    /* set receive buffer pointer */
    write_control_register(REG_ERXRDPTL, LOW(RXBUFFER_START));
    write_control_register(REG_ERXRDPTH, HIGH(RXBUFFER_START));
    write_control_register(REG_ERXWRPTL, LOW(RXBUFFER_START));
    write_control_register(REG_ERXWRPTH, HIGH(RXBUFFER_START));

    /* init next packet pointer */
    enc28j60_next_packet_pointer = RXBUFFER_START;

    /* bring MAC out of reset */
    bit_field_clear(REG_MACON2, _BV(MARST));

    /* enable MAC to receive frames (configure full-duplex flow control,
     * if using full-duplex mode) */
    bit_field_set(REG_MACON1, _BV(MARXEN) | _BV(TXPAUS) | _BV(RXPAUS));

    /* auto-pad to 60 bytes, enable automatic crc generation
     * and frame length checking */
    bit_field_set(REG_MACON3, _BV(PADCFG0) | _BV(TXCRCEN) | _BV(FRMLNEN));

    /* set full-duplex */
    write_phy(PHY_PHCON1, FULL_DUPLEX * _BV(PDPXMD) );

    /* read PHCON1 to check if full-duplex is enabled */
    if (read_phy(PHY_PHCON1) & _BV(PDPXMD)) {

        bit_field_set(REG_MACON3, _BV(FULDPX));

        /* configure inter-gap register with default value (0x12) from datasheet
         * for half-duplex operation */
        write_control_register(REG_MABBIPG, 0x12);

        /* configure other gap registers */
        write_control_register(REG_MAIPGL, 0x12);
        write_control_register(REG_MAIPGH, 0x0C);

    } else {

        bit_field_clear(REG_MACON3, _BV(FULDPX));

        /* configure inter-gap register with default value (0x15) from datasheet
         * for full-duplex operation */
        write_control_register(REG_MABBIPG, 0x15);

        /* configure other gap registers */
        write_control_register(REG_MAIPGL, 0x12);
        write_control_register(REG_MAIPGH, 0x0C);

    }

    /* write maximum frame length */
    write_control_register(REG_MAMXFLL, LOW(MAX_FRAME_LENGTH));
    write_control_register(REG_MAMXFLH, HIGH(MAX_FRAME_LENGTH));

    /* program the local mac address */
    write_control_register(REG_MAADR5, uip_ethaddr.addr[0]);
    write_control_register(REG_MAADR4, uip_ethaddr.addr[1]);
    write_control_register(REG_MAADR3, uip_ethaddr.addr[2]);
    write_control_register(REG_MAADR2, uip_ethaddr.addr[3]);
    write_control_register(REG_MAADR1, uip_ethaddr.addr[4]);
    write_control_register(REG_MAADR0, uip_ethaddr.addr[5]);

    /* configure leds: led a link status and receive activity, led b transmit activity */
    write_phy(PHY_PHLCON, _BV(STRCH) | _BV(LACFG3) | _BV(LACFG2) | _BV(LBCFG0));

    /* enable interrupts */
    write_control_register(REG_EIE, _BV(INTIE) | _BV(LINKIE) | _BV(PKTIE) | _BV(TXIE) | _BV(TXERIF) | _BV(RXERIF));
    /* do additional steps to enable link change interrupt */
    write_phy(PHY_PHIE, _BV(PGEIE) | _BV(PLINKIE));

    /* set filters */

    /* enable receiver */
    bit_field_set(REG_ECON1, _BV(ECON1_RXEN));

    /* set auto-increment bit */
    bit_field_set(REG_ECON2, _BV(AUTOINC));

} /* }}} */

void switch_bank(uint8_t bank)
/* {{{ */ {

    bit_field_clear(REG_ECON1, _BV(ECON1_BSEL0) | _BV(ECON1_BSEL1));
    bit_field_set(REG_ECON1, (bank & BANK_MASK));

    enc28j60_current_bank = bank;

} /* }}} */

/* dump out all the interesting registers
 * (copied from avrlib) */
#ifdef DEBUG_ENC28J60
void dump_debug_registers(void)
/* {{{ */ {

    uart_puts_P("RevID: 0x");
    uart_puthexbyte(read_control_register(REG_EREVID));
    uart_puts_P("\r\n");

    uart_puts_P("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n");
    uart_puts_P("       0x");
    uart_puthexbyte(read_control_register(REG_ECON1));
    uart_puts_P("  0x");
    uart_puthexbyte(read_control_register(REG_ECON2));
    uart_puts_P("  0x");
    uart_puthexbyte(read_control_register(REG_ESTAT));
    uart_puts_P("   0x");
    uart_puthexbyte(read_control_register(REG_EIR));
    uart_puts_P(" 0x");
    uart_puthexbyte(read_control_register(REG_EIE));
    uart_puts_P("\r\n");

    uart_puts_P("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\r\n");
    uart_puts_P("        0x");
    uart_puthexbyte(read_control_register(REG_MACON1));
    uart_puts_P("    0x");
    uart_puthexbyte(read_control_register(REG_MACON2));
    uart_puts_P("    0x");
    uart_puthexbyte(read_control_register(REG_MACON3));
    uart_puts_P("    0x");
    uart_puthexbyte(read_control_register(REG_MACON4));
    uart_puts_P("   ");
    uart_puthexbyte(read_control_register(REG_MAADR5));
    uart_puthexbyte(read_control_register(REG_MAADR4));
    uart_puthexbyte(read_control_register(REG_MAADR3));
    uart_puthexbyte(read_control_register(REG_MAADR2));
    uart_puthexbyte(read_control_register(REG_MAADR1));
    uart_puthexbyte(read_control_register(REG_MAADR0));
    uart_puts_P("\r\n");

    uart_puts_P("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n");
    uart_puts_P("       0x");
    uart_puthexbyte(read_control_register(REG_ERXSTH));
    uart_puthexbyte(read_control_register(REG_ERXSTL));
    uart_puts_P(" 0x");
    uart_puthexbyte(read_control_register(REG_ERXNDH));
    uart_puthexbyte(read_control_register(REG_ERXNDL));
    uart_puts_P("  0x");
    uart_puthexbyte(read_control_register(REG_ERXWRPTH));
    uart_puthexbyte(read_control_register(REG_ERXWRPTL));
    uart_puts_P("  0x");
    uart_puthexbyte(read_control_register(REG_ERXRDPTH));
    uart_puthexbyte(read_control_register(REG_ERXRDPTL));
    uart_puts_P("   0x");
    uart_puthexbyte(read_control_register(REG_ERXFCON));
    uart_puts_P("    0x");
    uart_puthexbyte(read_control_register(REG_EPKTCNT));
    uart_puts_P("  0x");
    uart_puthexbyte(read_control_register(REG_MAMXFLH));
    uart_puthexbyte(read_control_register(REG_MAMXFLL));
    uart_puts_P("\r\n");

    uart_puts_P("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n");
    uart_puts_P("       0x");
    uart_puthexbyte(read_control_register(REG_ETXSTH));
    uart_puthexbyte(read_control_register(REG_ETXSTL));
    uart_puts_P(" 0x");
    uart_puthexbyte(read_control_register(REG_ETXNDH));
    uart_puthexbyte(read_control_register(REG_ETXNDL));
    uart_puts_P("   0x");
    uart_puthexbyte(read_control_register(REG_MACLCON1));
    uart_puts_P("     0x");
    uart_puthexbyte(read_control_register(REG_MACLCON2));
    uart_puts_P("     0x");
    uart_puthexbyte(read_control_register(REG_MAPHSUP));
    uart_puts_P("\r\n");

    uart_puts_P("DMA  : EDMAST EDMAND\r\n");
    uart_puts_P("       0x");
    uart_puthexbyte(read_control_register(REG_EDMASTH));
    uart_puthexbyte(read_control_register(REG_EDMASTL));
    uart_puts_P(" 0x");
    uart_puthexbyte(read_control_register(REG_EDMANDH));
    uart_puthexbyte(read_control_register(REG_EDMANDL));
    uart_puts_P("\r\n");
} /* }}} */
#endif


