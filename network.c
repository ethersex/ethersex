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

#include <avr/eeprom.h>
#include <util/crc16.h>

#include "network.h"
#include "config.h"
#include "uart.h"
#include "eeprom.h"

#include "uip.h"
#include "uip_arp.h"

#define interrupt_occured() (!(INT_PIN & _BV(INT_PIN_NAME)))
#define wol_interrupt_occured() (!(WOL_PIN & _BV(WOL_PIN_NAME)))

void network_init(void)
/* {{{ */ {

    uip_init();
    uip_arp_init();

    uint8_t crc = 0;
    uint8_t *config = (uint8_t *)&eeprom_config;

    for (uint8_t i = 0; i < sizeof(struct eeprom_config_t) - 1; i++) {
        crc = _crc_ibutton_update(crc, eeprom_read_byte(config++));
    }

    uint8_t config_crc = eeprom_read_byte(&eeprom_config.crc);
    uip_ipaddr_t ipaddr;

    if (crc != config_crc) {
#ifdef DEBUG
        uart_puts_P("net: crc mismatch: 0x");
        uart_puthexbyte(crc);
        uart_puts_P(" != 0x");
        uart_puthexbyte(config_crc);
        uart_puts_P(" loading default settings\r\n");
#endif

        uip_ethaddr.addr[0] = 0xAC;
        uip_ethaddr.addr[1] = 0xDE;
        uip_ethaddr.addr[2] = 0x48;
        uip_ethaddr.addr[3] = 0xFD;
        uip_ethaddr.addr[4] = 0x0F;
        uip_ethaddr.addr[5] = 0xD1;

        uip_ipaddr(ipaddr, 10,0,0,1);
        uip_sethostaddr(ipaddr);
        uip_ipaddr(ipaddr, 255,255,255,0);
        uip_setnetmask(ipaddr);
    } else {

        uint8_t ip[4];

        /* load config settings */

        /* mac */
        for (uint8_t i = 0; i < 6; i++)
            uip_ethaddr.addr[i] = eeprom_read_byte(&eeprom_config.mac[i]);

        /* ip */
        for (uint8_t i = 0; i < 4; i++)
            ip[i] = eeprom_read_byte(&eeprom_config.ip[i]);

        uip_ipaddr(ipaddr, ip[0], ip[1], ip[2], ip[3]);
        uip_sethostaddr(ipaddr);

        /* netmask */
        for (uint8_t i = 0; i < 4; i++)
            ip[i] = eeprom_read_byte(&eeprom_config.netmask[i]);

        uip_ipaddr(ipaddr, ip[0], ip[1], ip[2], ip[3]);
        uip_setnetmask(ipaddr);

        /* gateway */
        for (uint8_t i = 0; i < 4; i++)
            ip[i] = eeprom_read_byte(&eeprom_config.gateway[i]);

        uip_ipaddr(ipaddr, ip[0], ip[1], ip[2], ip[3]);
        uip_setdraddr(ipaddr);

    }

    init_enc28j60();

} /* }}} */

void enc28j60_process_interrupts(void)
/* {{{ */ {

    /* also check packet counter, see errata #6 */
#   ifdef ENC28J60_REV4_WORKAROUND
    uint8_t pktcnt = read_control_register(REG_EPKTCNT);
#   endif

    /* if no interrupt occured and less than 5 packets are in the receive
     * buffer, return */
    if ( ! (interrupt_occured()
#   ifdef ENC28J60_REV4_WORKAROUND
                || pktcnt > 5
#   endif
           ) )
        return;

#   if defined(ENC28J60_REV4_WORKAROUND) && defined(DEBUG_REV4_WORKAROUND)
    if (pktcnt > 5)
        uart_puts_P("net: BUG: pktcnt > 5\r\n");
#   endif

    /* read interrupt register */
    uint8_t EIR = read_control_register(REG_EIR);

    /* clear global interrupt flag */
    bit_field_clear(REG_EIE, _BV(INTIE));

    /* check if some interrupts occured */
    if (EIR != 0) {

        /* check each interrupt flag the interrupt is activated for, and clear it
         * if neccessary */

#ifdef DEBUG_INTERRUPT
        uart_puts_P("net: controller interrupt, EIR = 0x");
        uart_puthexbyte(EIR);
        uart_puts_P(" = ");
        if (EIR & _BV(LINKIF))
            uart_putc('L');
        if (EIR & _BV(TXIF))
            uart_putc('T');
        if (EIR & _BV(PKTIF))
            uart_putc('P');
        if (EIR & _BV(RXERIF))
            uart_putc('r');
        if (EIR & _BV(TXERIF))
            uart_putc('t');
        uart_eol();
#endif

        /* link change flag */
        if (EIR & _BV(LINKIF)) {
            /* clear interrupt flag */
            read_phy(PHY_PHIR);

            /* read new link state */
            uint8_t link_state = (read_phy(PHY_PHSTAT2) & _BV(LSTAT)) > 0;

#ifdef DEBUG
            if (link_state) {
                uart_puts_P("net: got link!\r\n");
            } else
                uart_puts_P("net: no link!\r\n");
#endif
        }

        /* packet transmit flag */
        if (EIR & _BV(TXIF)) {

#if 0
            /* clear send_buffer, if not waiting for ack */
            for (uint8_t i = 0; i < NET_SEND_BUFFER_SIZE; i++) {

                /* if this buffer is transmitting, remove flag */
                if (global_net.send_buffer[i].flags & _BV(SEND_BUFFER_TRANSMITTING)) {
                    global_net.send_buffer[i].flags &= ~_BV(SEND_BUFFER_TRANSMITTING);

                    /* if this buffer is not waiting for an ack, clear it */
                    if ( !(global_net.send_buffer[i].flags & _BV(SEND_BUFFER_WAIT_ACK))) {
#ifdef DEBUG_INTERRUPT
                        uart_puts_P("net: found transmitting send buffer 0x");
                        uart_puthexbyte(i);
                        uart_puts_P(", cleaning up\r\n");
#endif
                        global_net.send_buffer[i].flags = 0;
                    }
                }
            }
#endif

#ifdef DEBUG
            uint8_t ESTAT = read_control_register(REG_ESTAT);

            if (ESTAT & _BV(TXABRT))
                uart_puts_P("net: packet transmit failed\r\n");
#endif
            /* clear flags */
            bit_field_clear(REG_EIR, _BV(TXIF));
            bit_field_clear(REG_ESTAT, _BV(TXABRT) | _BV(LATECOL) );
        }

        /* packet receive flag */
        if (EIR & _BV(PKTIF)) {

            process_packet();
        }

        /* receive error */
        if (EIR & _BV(RXERIF)) {
            uart_puts_P("net: receive error!\r\n");

            bit_field_clear(REG_EIR, _BV(RXERIF));

#ifdef ENC28J60_REV4_WORKAROUND
            init_enc28j60();
#endif

        }

        /* transmit error */
        if (EIR & _BV(TXERIF)) {
#ifdef DEBUG
            uart_puts_P("net: transmit error!\r\n");
#endif

            bit_field_clear(REG_EIR, _BV(TXERIF));
        }

    }

    /* set global interrupt flag */
    bit_field_set(REG_EIE, _BV(INTIE));


} /* }}} */

void process_packet(void)
/* {{{ */ {

    /* if there is a packet to process */
    if (read_control_register(REG_EPKTCNT) == 0)
        return;

#   ifdef DEBUG_NET
    uart_puts_P("net: packet received\r\n");
#   endif

    /* read next packet pointer */
    set_read_buffer_pointer(enc28j60_next_packet_pointer);
    enc28j60_next_packet_pointer = read_buffer_memory() | (read_buffer_memory() << 8);

    /* read receive status vector */
    struct receive_packet_vector_t rpv;
    uint8_t *p = (uint8_t *)&rpv;

    for (uint8_t i = 0; i < sizeof(struct receive_packet_vector_t); i++)
        *p++ = read_buffer_memory();

    /* decrement rpv received_packet_size by 4, because the 4 byte CRC checksum is counted */
    rpv.received_packet_size -= 4;

    /* check size */
    if (rpv.received_packet_size > MAX_FRAME_LENGTH
            || rpv.received_packet_size < sizeof(struct uip_eth_hdr)
            || rpv.received_packet_size > UIP_BUFSIZE) {
#       ifdef DEBUG
        uart_puts_P("net: packet too large or too small for an ethernet header: ");
        uart_puthexbyte(HIGH(rpv.received_packet_size));
        uart_puthexbyte( LOW(rpv.received_packet_size));
#       endif
        return;
    }

    /* store packet start pointer for checksum calculation */
    //global_net.packet_start_pointer = get_read_buffer_pointer();

    /* read packet */
    p = uip_buf;
    for (uint16_t i = 0; i < rpv.received_packet_size; i++)
        *p++ = read_buffer_memory();

    uip_len = rpv.received_packet_size;

    /* process packet */
    struct uip_eth_hdr *packet = (struct uip_eth_hdr *)&uip_buf;
    switch (ntohs(packet->type)) {

        /* process arp packet */
        case UIP_ETHTYPE_ARP:
#           ifdef DEBUG_NET
            uart_puts_P("net: arp packet received\r\n");
#           endif
            uip_arp_arpin();

            /* if there is a packet to send, send it now */
            if (uip_len > 0)
                transmit_packet();

            break;

        /* process ip packet */
        case UIP_ETHTYPE_IP:
#           ifdef DEBUG_NET
            uart_puts_P("net: ip packet received\r\n");
#           endif
            uip_arp_ipin();
            uip_input();

            /* if there is a packet to send, send it now */
            if (uip_len > 0) {

                /* check if an arp request has to be send */
                uip_arp_out();
                transmit_packet();
            }

            break;
#       ifdef DEBUG_UNKNOWN_PACKETS
        default:
            /* debug output */
            uart_puts_P("net: unknown packet, ");
            uart_puts_mac(&packet->src);
            uart_puts_P(" -> ");
            uart_puts_mac(&packet->dest);
            uart_puts_P(", type 0x");
            uart_puthexbyte(HIGH(ntohs(packet->type)));
            uart_puthexbyte( LOW(ntohs(packet->type)));
            uart_eol();
            break;
#       endif
    }

    /* advance receive read pointer, ensuring that an odd value is programmed
     * (next_receive_packet_pointer is always even), see errata #13 */
    if ( (enc28j60_next_packet_pointer - 1) < RXBUFFER_START
            || (enc28j60_next_packet_pointer - 1) > RXBUFFER_END) {

        write_control_register(REG_ERXRDPTL, LOW(RXBUFFER_END));
        write_control_register(REG_ERXRDPTH, HIGH(RXBUFFER_END));

    } else {

        write_control_register(REG_ERXRDPTL, LOW(enc28j60_next_packet_pointer - 1));
        write_control_register(REG_ERXRDPTH, HIGH(enc28j60_next_packet_pointer - 1));

    }

    /* decrement packet counter */
    bit_field_set(REG_ECON2, _BV(PKTDEC));

} /* }}} */

void transmit_packet(void)
/* {{{ */ {

    /* wait for any transmits to end, with timeout */
    uint8_t timeout = 100;
    while (read_control_register(REG_ECON1) & _BV(ECON1_TXRTS) && timeout-- > 0);

    if (timeout == 0) {

#       ifdef DEBUG
        uart_puts_P("net: timeout waiting for TXRTS!\r\n");
#       endif

    } else {

        uint16_t start_pointer = TXBUFFER_START;

        /* set send control registers */
        write_control_register(REG_ETXSTL, LOW(start_pointer));
        write_control_register(REG_ETXSTH, HIGH(start_pointer));

        write_control_register(REG_ETXNDL, LOW(start_pointer + uip_len));
        write_control_register(REG_ETXNDH, HIGH(start_pointer + uip_len));

        /* set pointer to beginning of tx buffer */
        set_write_buffer_pointer(start_pointer);

        /* write override byte */
        write_buffer_memory(0);

        /* write data */
        for (uint16_t i = 0; i < uip_len; i++)
            write_buffer_memory(uip_buf[i]);

#       if 0
        uart_puts_P("net: sending packet in from buffer 0x");
        uart_puthexbyte(i);
        uart_eol();
        uint16_t read_buffer_pointer = get_read_buffer_pointer();
        set_read_buffer_pointer(global_net.send_buffer[i].start_pointer+1);
        uart_puts_P("net: packet goes to: ");
        for (uint8_t j = 0; j < 6; j++)
            uart_puthexbyte(read_buffer_memory());
        uart_puts_P(" from: ");
        for (uint8_t j = 0; j < 6; j++)
            uart_puthexbyte(read_buffer_memory());
        uart_eol();
        set_read_buffer_pointer(read_buffer_pointer);
#       endif

#       ifdef ENC28J60_REV4_WORKAROUND
        /* reset transmit hardware, see errata #12 */
        bit_field_set(REG_ECON1, _BV(ECON1_TXRST));
        bit_field_clear(REG_ECON1, _BV(ECON1_TXRST));
#       endif

        /* transmit packet */
        bit_field_set(REG_ECON1, _BV(ECON1_TXRTS));

    }

} /* }}} */
