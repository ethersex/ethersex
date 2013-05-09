/*
 *
 *          enc28j60 api
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/ethernet/enc28j60.h"
#include "network.h"
#include "core/spi.h"
#include "core/bit-macros.h"

/* global variables */
uint8_t enc28j60_current_bank = 0;
int16_t enc28j60_next_packet_pointer;

#define DEBUG_REV6_WORKAROUND
#ifdef DEBUG_REV6_WORKAROUND
 uint8_t macon1 = 0;
 uint8_t macon3 = 0;
 uint8_t debug_guard = 0; /* if true, ENC28j60 is in reset process */
 
 #define DEBUG_GUARD debug_guard
 #define MACON1_PP macon1++
 #define MACON3_PP macon3++
#endif



/* module local macros */
#ifdef RFM12_IP_SUPPORT
/* RFM12 uses interrupts which do SPI interaction, therefore
   we have to disable interrupts if support is enabled */
#  define cs_low()  uint8_t sreg = SREG; cli(); PIN_CLEAR(SPI_CS_NET); 
#  define cs_high() PIN_SET(SPI_CS_NET); SREG = sreg;
#else
#  define cs_low()  PIN_CLEAR(SPI_CS_NET)
#  define cs_high() PIN_SET(SPI_CS_NET)
#endif


uint8_t read_control_register(uint8_t address)
{

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < KEY_REGISTERS &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode and address */
    spi_send(CMD_RCR | (address & REGISTER_ADDRESS_MASK));

    /* read data */
    uint8_t data = spi_send(0);

    /* if this is a register in MAC or MII (when MSB is set),
     * only a dummy byte has been read so far, read real data now */
    if (address & _BV(7)) {
        data = spi_send(0);
    }

    /* release device */
    cs_high();

    return data;

}

uint8_t read_buffer_memory(void)
{

    /* aquire device */
    cs_low();

    /* send opcode */
    spi_send(CMD_RBM);

    /* read data */
    uint8_t data = spi_send(0);

    /* release device */
    cs_high();

    return data;

}

void write_control_register(uint8_t address, uint8_t data)
{

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < KEY_REGISTERS &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode */
    spi_send(CMD_WCR | (address & REGISTER_ADDRESS_MASK) );

    /* send data */
    spi_send(data);

    /* release device */
    cs_high();

}

void write_buffer_memory(uint8_t data)
{

    /* aquire device */
    cs_low();

    /* send opcode */
    spi_send(CMD_WBM);

    /* send data */
    spi_send(data);

    /* release device */
    cs_high();

}

void bit_field_modify(uint8_t address, uint8_t mask, uint8_t opcode)
{

    /* change to appropiate bank */
    if ( (address & REGISTER_ADDRESS_MASK) < KEY_REGISTERS &&
         ((address & REGISTER_BANK_MASK) >> 5) != (enc28j60_current_bank))
        switch_bank((address & REGISTER_BANK_MASK) >> 5);

    /* aquire device */
    cs_low();

    /* send opcode */
    spi_send(opcode | (address & REGISTER_ADDRESS_MASK) );

    /* send data */
    spi_send(mask);

    /* release device */
    cs_high();

}

void noinline set_read_buffer_pointer(uint16_t address)
{

    write_control_register(REG_ERDPTL, LO8(address));
    write_control_register(REG_ERDPTH, HI8(address));

}

uint16_t noinline get_read_buffer_pointer(void)
{

    return (read_control_register(REG_ERDPTL) | (read_control_register(REG_ERDPTH) << 8));

}

void noinline set_write_buffer_pointer(uint16_t address)
{

    write_control_register(REG_EWRPTL, LO8(address));
    write_control_register(REG_EWRPTH, HI8(address));

}

uint16_t noinline get_write_buffer_pointer(void)
{

    return (read_control_register(REG_EWRPTL) | (read_control_register(REG_EWRPTH) << 8));

}

uint16_t read_phy(uint8_t address)
{

    write_control_register(REG_MIREGADR, address);
    bit_field_set(REG_MICMD, _BV(MIIRD));

    /* wait for this operation to complete */
    while(read_control_register(REG_MISTAT) & _BV(BUSY));

    /* stop reading */
    bit_field_clear(REG_MICMD, _BV(MIIRD));

    return (read_control_register(REG_MIRDL) | (read_control_register(REG_MIRDH) << 8));

}

void write_phy(uint8_t address, uint16_t data)
{

    /* set address */
    write_control_register(REG_MIREGADR, address);

    /* set data */
    write_control_register(REG_MIWRL, LO8(data));
    write_control_register(REG_MIWRH, HI8(data));

    /* start writing and wait */
    while(read_control_register(REG_MISTAT) & _BV(BUSY));

}

void reset_controller(void)
{

    /* aquire device */
    cs_low();

    /* send opcode */
    spi_send(CMD_RESET);

    /* wait until the controller is ready */
#ifdef ENC28J60_REV5_WORKAROUND
    _delay_ms (2);  /* see errata #2: Module Reset */
#else
    while (!(read_control_register(REG_ESTAT) & _BV(CLKRDY)));
#endif

    /* release device */
    cs_high();

}

void reset_rx(void)
{

    /* reset rx logic */
    bit_field_set(REG_ECON1, _BV(ECON1_RXRST));
    _delay_loop_2(40000);
    bit_field_clear(REG_ECON1, _BV(ECON1_RXRST));

    /* re-enable rx */
    bit_field_set(REG_ECON1, _BV(ECON1_RXEN));

}

void init_enc28j60(void)
{
#ifdef DEBUG_REV6_WORKAROUND
    debug_guard=0xff;
#endif

    reset_controller();
    
#if CONF_ENC_ECOCON != ECOCON_UNSET
    /* set ECOCON (CLKOUT prescaler) */
    write_control_register(REG_ECOCON, CONF_ENC_ECOCON);
#endif

    /* set receive buffer to span from 0 to 4kb */
    write_control_register(REG_ERXSTL, LO8(RXBUFFER_START));
    write_control_register(REG_ERXSTH, HI8(RXBUFFER_START));
    write_control_register(REG_ERXNDL, LO8(RXBUFFER_END));
    write_control_register(REG_ERXNDH, HI8(RXBUFFER_END));

    /* set transmit buffer start at 4kb */
    write_control_register(REG_ETXSTL, LO8(TXBUFFER_START));
    write_control_register(REG_ETXSTH, HI8(TXBUFFER_START));

    /* set receive buffer pointer */
    write_control_register(REG_ERXRDPTL, LO8(RXBUFFER_START));
    write_control_register(REG_ERXRDPTH, HI8(RXBUFFER_START));
    write_control_register(REG_ERXWRPTL, LO8(RXBUFFER_START));
    write_control_register(REG_ERXWRPTH, HI8(RXBUFFER_START));

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
    write_phy(PHY_PHCON1, NET_FULL_DUPLEX * _BV(PDPXMD) );

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

    /* write maximum frame length, append 4 bytes for crc (added by enc28j60) */
    write_control_register(REG_MAMXFLL, LO8(NET_MAX_FRAME_LENGTH + 4));
    write_control_register(REG_MAMXFLH, HI8(NET_MAX_FRAME_LENGTH + 4));

    /* program the local mac address */
    write_control_register(REG_MAADR5, uip_ethaddr.addr[0]);
    write_control_register(REG_MAADR4, uip_ethaddr.addr[1]);
    write_control_register(REG_MAADR3, uip_ethaddr.addr[2]);
    write_control_register(REG_MAADR2, uip_ethaddr.addr[3]);
    write_control_register(REG_MAADR1, uip_ethaddr.addr[4]);
    write_control_register(REG_MAADR0, uip_ethaddr.addr[5]);

    /* receive broadcast, multicast and unicast packets */
    write_control_register(REG_ERXFCON, _BV(BCEN) | _BV(MCEN) | _BV(UCEN));

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

#ifdef DEBUG_REV6_WORKAROUND
    debug_guard=0x0;
#endif


}

void switch_bank(uint8_t bank)
{

    bit_field_clear(REG_ECON1, _BV(ECON1_BSEL0) | _BV(ECON1_BSEL1));
    bit_field_set(REG_ECON1, (bank & BANK_MASK));

    enc28j60_current_bank = bank;

}

void enc28j60_periodic(void) 
{
    uint8_t mask = _BV(PADCFG0) | _BV(TXCRCEN) | _BV(FRMLNEN);
#ifdef DEBUG_REV6_WORKAROUND
  if (!DEBUG_GUARD) {
    if (   (read_control_register(REG_MACON3) & mask) != mask ) {
 	MACON1_PP;	
	init_enc28j60();
	}
    if (   (read_control_register(REG_MACON1))        != 0x0D  ) {
	MACON3_PP;
	init_enc28j60();
    }
  }
#else
    if (   ((read_control_register(REG_MACON3) & mask) != mask  )
        || ( read_control_register(REG_MACON1)         != 0x0D  ) ) {
        init_enc28j60();
    }
#endif
}

/* dump out all the interesting registers
 * (mainly copied from avrlib) */
#ifdef DEBUG_ENC28J60
int16_t parse_cmd_enc_dump(char *cmd, char *output, uint16_t len)
{
    (void) cmd;
    (void) output;
    (void) len;

    debug_printf("RevID: 0x%02x\n", read_control_register(REG_EREVID));

    debug_printf("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\n");
    debug_printf("       0x%02x  0x%02x  0x%02x   0x%02x 0x%02x\n",
        read_control_register(REG_ECON1),
        read_control_register(REG_ECON2),
        read_control_register(REG_ESTAT),
        read_control_register(REG_EIR),
        read_control_register(REG_EIE));

    debug_printf("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\n");
    debug_printf("        0x%02x    0x%02x    0x%02x    0x%02x  %02x:%02x:%02x:%02x:%02x:%02x\n",
        read_control_register(REG_MACON1),
        read_control_register(REG_MACON2),
        read_control_register(REG_MACON3),
        read_control_register(REG_MACON4),
        read_control_register(REG_MAADR5),
        read_control_register(REG_MAADR4),
        read_control_register(REG_MAADR3),
        read_control_register(REG_MAADR2),
        read_control_register(REG_MAADR1),
        read_control_register(REG_MAADR0));

    debug_printf("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\n");
    debug_printf("      0x%02x%02x 0x%02x%02x   0x%02x%02x  0x%02x%02x    0x%02x    0x%02x 0x%02x%02x\n",
        read_control_register(REG_ERXSTH),
        read_control_register(REG_ERXSTL),
        read_control_register(REG_ERXNDH),
        read_control_register(REG_ERXNDL),
        read_control_register(REG_ERXWRPTH),
        read_control_register(REG_ERXWRPTL),
        read_control_register(REG_ERXRDPTH),
        read_control_register(REG_ERXRDPTL),
        read_control_register(REG_ERXFCON),
        read_control_register(REG_EPKTCNT),
        read_control_register(REG_MAMXFLH),
        read_control_register(REG_MAMXFLL));

    debug_printf("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\n");
    debug_printf("       0x%02x%02x 0x%02x%02x   0x%02x     0x%02x     0x%02x\n",
        read_control_register(REG_ETXSTH),
        read_control_register(REG_ETXSTL),
        read_control_register(REG_ETXNDH),
        read_control_register(REG_ETXNDL),
        read_control_register(REG_MACLCON1),
        read_control_register(REG_MACLCON2),
        read_control_register(REG_MAPHSUP));

    debug_printf("DMA  : EDMAST EDMAND\n");
    debug_printf("       0x%02x%02x 0x%02x%02x\n",
        read_control_register(REG_EDMASTH),
        read_control_register(REG_EDMASTL),
        read_control_register(REG_EDMANDH),
        read_control_register(REG_EDMANDL));

#ifdef DEBUG_REV6_WORKAROUND
    debug_printf("debug: macon1= %d, macon3= %d \n", macon1, macon3);
#endif

    return 0;
}
#endif

/*
  -- Ethersex META --
  header(hardware/ethernet/enc28j60.h)
  net_init(init_enc28j60)
  mainloop(network_process)
  timer(50, enc28j60_periodic())
*/
