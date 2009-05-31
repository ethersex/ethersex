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
#include "core/eeprom.h"

/* global variables */
uint8_t enc28j60_current_bank = 0;
int16_t enc28j60_next_packet_pointer;

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
    _delay_ms (2);
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

    reset_controller();

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

}

void switch_bank(uint8_t bank)
{

    bit_field_clear(REG_ECON1, _BV(ECON1_BSEL0) | _BV(ECON1_BSEL1));
    bit_field_set(REG_ECON1, (bank & BANK_MASK));

    enc28j60_current_bank = bank;

}

/* dump out all the interesting registers
 * (mainly copied from avrlib) */
#ifdef DEBUG_ENC28J60
void dump_debug_registers(void)
{

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
}
#endif




void
network_config_load (void)
{
    /* load settings from eeprom */
#ifdef EEPROM_SUPPORT
  eeprom_restore(mac, uip_ethaddr.addr, 6);
#else 
  memcpy_P(uip_ethaddr.addr, PSTR(CONF_ETHERRAPE_MAC), 6);
#endif

#if defined(BOOTP_SUPPORT)				\
    || (IPV6_SUPPORT && !defined(IPV6_STATIC_SUPPORT))
    return;

#else

    uip_ipaddr_t ip;
    (void) ip;		/* Keep GCC quiet. */

    /* Configure the IP address. */
#ifdef EEPROM_SUPPORT
    /* Please Note: ip and &ip are NOT the same (cpp hell) */
    eeprom_restore_ip(ip, &ip);
#else
    set_CONF_ETHERRAPE_IP(&ip);
#endif
    uip_sethostaddr(&ip);


    /* Configure prefix length (IPv6). */
#ifdef IPV6_SUPPORT
    uip_setprefixlen(CONF_ENC_IP6_PREFIX_LEN);
#endif


#ifdef IPV4_SUPPORT
    /* Configure the netmask (IPv4). */
#ifdef EEPROM_SUPPORT
    /* Please Note: ip and &ip are NOT the same (cpp hell) */
    eeprom_restore_ip(netmask, &ip);
#else
    set_CONF_ETHERRAPE_IP4_NETMASK(&ip);
#endif
    uip_setnetmask(&ip);
#endif  /* IPV4_SUPPORT */

    /* Configure the default gateway  */
#ifdef EEPROM_SUPPORT
    /* Please Note: ip and &ip are NOT the same (cpp hell) */
    eeprom_restore_ip(gateway, &ip);
#else
    set_CONF_ETHERRAPE_GATEWAY(&ip);
#endif
    uip_setdraddr(&ip);
#endif	/* No autoconfiguration. */
}


/*
  -- Ethersex META --
  header(hardware/ethernet/enc28j60.h)
  net_init(init_enc28j60)
  mainloop(network_process)
  timer(1, `
#       if UIP_CONF_IPV6
        if (counter == 5) {
            // Send a router solicitation every 10 seconds, as long
            // as we only got a link local address.  First time one
            // second after boot 
#           ifndef IPV6_STATIC_SUPPORT
            if(((u16_t *)(uip_hostaddr))[0] == HTONS(0xFE80)) {
                uip_router_send_solicitation();
                transmit_packet();
            }
#           endif
        }
#       endif // UIP_CONF_IPV6 
')
*/
