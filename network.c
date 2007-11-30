/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
#include <string.h>
#include <avr/pgmspace.h>

#include "network.h"
#include "config.h"
#include "eeprom.h"
#include "ipv6.h"
#include "bit-macros.h"
#include "net/handler.h"
#include "dns/resolv.h"

#include "debug.h"

#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/uip_neighbor.h"

#ifndef ENC28J60_POLL
    #define interrupt_occured() (!(INT_PIN & _BV(INT_PIN_NAME)))
    #define wol_interrupt_occured() (!(WOL_PIN & _BV(WOL_PIN_NAME)))
#else
    #define interrupt_occured() 0
    #define wol_interrupt_occured() 0
#endif

/* prototypes */
void process_packet(void);

void network_init(void)
/* {{{ */ {
    uip_stack_set_active(STACK_MAIN);
    uip_init();

#ifdef OPENVPN_SUPPORT
    uip_stack_set_active(STACK_OPENVPN);
    openvpn_init();
#endif

#ifdef ENC28J60_SUPPORT
#if UIP_CONF_IPV6
    uip_neighbor_init();
#else
    uip_arp_init();
#endif
#endif /* ENC28J60_SUPPORT */

    uip_stack_set_active(STACK_MAIN);

    uip_ipaddr_t ipaddr;

    /* use uip buffer as generic space here, since when this function is called,
     * no network packets will be processed */
    void *buf = uip_buf;

    /* load base network settings */
#   ifdef DEBUG_NET_CONFIG
    debug_printf("net: loading base network settings\n");
#   endif

#   ifdef ENC28J60_SUPPORT
#   if UIP_CONF_IPV6 && (UIP_CONF_IPV6_LLADDR || !defined(OPENVPN_SUPPORT))
    uip_ip6autoconfig(0xFE80, 0x0000, 0x0000, 0x0000);
#   if UIP_CONF_IPV6_LLADDR
    uip_ipaddr_copy(uip_lladdr, uip_hostaddr);
#   endif
#   endif
#   endif /* ENC28J60_SUPPORT */

    /* use global network packet buffer for configuration */
    eeprom_read_block(buf, EEPROM_CONFIG_BASE, sizeof(struct eeprom_config_base_t));

    /* checksum */
    uint8_t checksum = crc_checksum(buf, sizeof(struct eeprom_config_base_t) - 1);

    struct eeprom_config_base_t *cfg_base = (struct eeprom_config_base_t *)buf;

    if (checksum != cfg_base->crc) {

        uip_ipaddr_t ip;

        debug_printf("net: crc mismatch: 0x%x != 0x%x, loading default settings\n",
            checksum, cfg_base->crc);

        /* load default settings and save to buffer */
        memcpy_P(uip_ethaddr.addr, PSTR(CONF_ETHERRAPE_MAC), 6);
        memcpy(&cfg_base->mac, uip_ethaddr.addr, 6);

#       if (!UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)) \
  || defined(OPENVPN_SUPPORT) || (UIP_CONF_IPV6 && !defined(ENC28J60_SUPPORT))
        CONF_ETHERRAPE_IP;
        uip_sethostaddr(ip);
#       ifndef BOOTLOADER_SUPPORT        
        memcpy(&cfg_base->ip, &ip, sizeof(uip_ipaddr_t));
#       endif
#       endif

#       if (!UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT))
        CONF_ETHERRAPE_IP4_NETMASK;
        uip_setnetmask(ip);
#       ifndef BOOTLOADER_SUPPORT        
        memcpy(&cfg_base->netmask, &ip, sizeof(uip_ipaddr_t));
#       endif

        CONF_ETHERRAPE_IP4_GATEWAY;
        uip_setdraddr(ip);
#       ifndef BOOTLOADER_SUPPORT        
        memcpy(&cfg_base->gateway, &ip, sizeof(uip_ipaddr_t));
#       endif
#       endif /* not UIP_CONF_IPV6 and not BOOTP */

#       if defined(DNS_SUPPORT) && !defined(BOOTP_SUPPORT)
        CONF_DNS_SERVER;
        memcpy(&cfg_base->dns_server, &ip, sizeof(uip_ipaddr_t));
        resolv_conf(&ip);
#       endif

#       ifndef BOOTLOADER_SUPPORT        
        /* calculate new checksum */
        checksum = crc_checksum(buf, sizeof(struct eeprom_config_base_t) - 1);
        cfg_base->crc = checksum;

        /* save config */
        eeprom_write_block(buf, EEPROM_CONFIG_BASE, sizeof(struct eeprom_config_base_t));
#       endif /* !BOOTLOADER_SUPPORT */

    } else {

        /* load settings from eeprom */
        memcpy(uip_ethaddr.addr, &cfg_base->mac, 6);

#       if (!UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)) \
  || defined(OPENVPN_SUPPORT)
        memcpy(&ipaddr, &cfg_base->ip, sizeof(uip_ipaddr_t));
        uip_sethostaddr(ipaddr);
#       endif

#       if !UIP_CONF_IPV6 && !defined(BOOTP_SUPPORT)
        memcpy(&ipaddr, &cfg_base->netmask, 4);
        uip_setnetmask(ipaddr);
        memcpy(&ipaddr, &cfg_base->gateway, 4);
        uip_setdraddr(ipaddr);
#       endif /* not UIP_CONF_IPV6 and not BOOTP */
        
        /* optimized version: FIXME: does this work?
        memcpy(uip_ethaddr.addr, &cfg_base->mac, 6);
        uip_sethostaddr(&cfg_base->ip);
        uip_sethostaddr(&cfg_base->netmask);
        uip_setdraddr(&cfg_base->gateway);
        */

#       if defined(DNS_SUPPORT) && !defined(BOOTP_SUPPORT)
        memcpy(&ipaddr, &cfg_base->dns_server, IPADDR_LEN);
        resolv_conf(&ipaddr);
#       endif
    }

#   if defined(DEBUG_NET_CONFIG) && !UIP_CONF_IPV6
    debug_printf("ip: %d.%d.%d.%d/%d.%d.%d.%d, gw: %d.%d.%d.%d\n",
        LO8(uip_hostaddr[0]), HI8(uip_hostaddr[0]), LO8(uip_hostaddr[1]), HI8(uip_hostaddr[1]),
        LO8(uip_netmask[0]), HI8(uip_netmask[0]), LO8(uip_netmask[1]), HI8(uip_netmask[1]),
        LO8(uip_draddr[0]), HI8(uip_draddr[0]), LO8(uip_draddr[1]), HI8(uip_draddr[1]));
#   endif

#   ifndef BOOTLOADER_SUPPORT
    /* load extended network settings */
#   ifdef DEBUG_NET_CONFIG
    debug_printf("net: loading extended network settings\n");
#   endif

    /* use global network packet buffer for configuration */
    eeprom_read_block(buf, EEPROM_CONFIG_EXT, sizeof(struct eeprom_config_ext_t));

    /* checksum */
    checksum = crc_checksum(buf, sizeof(struct eeprom_config_ext_t) - 1);

    struct eeprom_config_ext_t *cfg_ext = (struct eeprom_config_ext_t *)buf;

    if (checksum != cfg_ext->crc) {
        debug_printf("net: ext crc mismatch: 0x%x != 0x%x, loading default settings\n",
                checksum, cfg_ext->crc);

        /* set defaults */
        memset(&cfg.sntp_server, 0, sizeof(uip_ipaddr_t));
        cfg.options.sntp = 0;

    } else {

        /* load settings */
        memcpy(&cfg.sntp_server, &cfg_ext->sntp_server, sizeof(uip_ipaddr_t));
        memcpy(&cfg.options, &cfg_ext->options, sizeof(global_options_t));

#       if defined(DEBUG_NET_CONFIG) && !UIP_CONF_IPV6
        if (cfg.options.sntp)
            debug_printf("cfg: sntp server is %d.%d.%d.%d\n",
                    LO8(cfg.sntp_server[0]), HI8(cfg.sntp_server[0]),
                    LO8(cfg.sntp_server[1]), HI8(cfg.sntp_server[1]));
#       endif

    }
#   endif /* !BOOTLOADER_SUPPORT */

    network_init_apps();

#   ifdef ENC28J60_SUPPORT
    init_enc28j60();
#   endif

} /* }}} */


#ifdef ENC28J60_SUPPORT
void network_process(void)
/* {{{ */ {

    /* also check packet counter, see errata #6 */
#   ifdef ENC28J60_REV4_WORKAROUND
    uint8_t pktcnt = read_control_register(REG_EPKTCNT);
#   endif

    /* if no interrupt occured and no packets are in the receive
     * buffer, return */
    if ( !interrupt_occured()
#   ifdef ENC28J60_REV4_WORKAROUND
                || pktcnt == 0
#   endif
           )
        return;

#   if defined(ENC28J60_REV4_WORKAROUND) && defined(DEBUG_REV4_WORKAROUND)
    if (pktcnt > 5)
        debug_printf("net: BUG: pktcnt > 5\n");
#   endif

    /* read interrupt register */
    uint8_t EIR = read_control_register(REG_EIR);

    /* clear global interrupt flag */
    bit_field_clear(REG_EIE, _BV(INTIE));

#ifdef DEBUG_INTERRUPT
    /* check if some interrupts occured */
    if (EIR != 0) {

        debug_printf("net: controller interrupt, EIR = 0x%02x\n", EIR);
        if (EIR & _BV(LINKIF))
            debug_printf("\t* Link\n");
        if (EIR & _BV(TXIF))
            debug_printf("\t* Tx\n");
        if (EIR & _BV(PKTIF))
            debug_printf("\t* Pkt\n");
        if (EIR & _BV(RXERIF))
            debug_printf("\t* rx error\n");
        if (EIR & _BV(TXERIF))
            debug_printf("\t* tx error\n");
    }
#endif

    /* check each interrupt flag the interrupt is activated for, and clear it
     * if neccessary */

    /* link change flag */
    if (EIR & _BV(LINKIF)) {

        /* clear interrupt flag */
        read_phy(PHY_PHIR);

        /* read new link state */
        uint8_t link_state = (read_phy(PHY_PHSTAT2) & _BV(LSTAT)) > 0;

        if (link_state) {
            debug_printf("net: got link!\n");
        } else
            debug_printf("net: no link!\n");

    }

    /* packet transmit flag */
    if (EIR & _BV(TXIF)) {

#ifdef DEBUG
        uint8_t ESTAT = read_control_register(REG_ESTAT);

        if (ESTAT & _BV(TXABRT))
            debug_printf("net: packet transmit failed\n");
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
        debug_printf("net: receive error!\n");

        bit_field_clear(REG_EIR, _BV(RXERIF));

#ifdef ENC28J60_REV4_WORKAROUND
        init_enc28j60();
#endif

    }

    /* transmit error */
    if (EIR & _BV(TXERIF)) {
#ifdef DEBUG
        debug_printf("net: transmit error!\n");
#endif

        bit_field_clear(REG_EIR, _BV(TXERIF));
    }

    /* set global interrupt flag */
    bit_field_set(REG_EIE, _BV(INTIE));

} /* }}} */
#endif /* ENC28J60_SUPPORT */


#ifdef ENC28J60_SUPPORT
void process_packet(void)
/* {{{ */ {

    /* if there is a packet to process */
    if (read_control_register(REG_EPKTCNT) == 0)
        return;

#   ifdef DEBUG_NET
    debug_printf("net: packet received\n");
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
    if (rpv.received_packet_size > NET_MAX_FRAME_LENGTH
            || rpv.received_packet_size < 14
            || rpv.received_packet_size > UIP_BUFSIZE) {
#       ifdef DEBUG
        debug_printf("net: packet too large or too small for an ethernet header: %d\n", rpv.received_packet_size);
#       endif
        return;
    }

    /* read packet */
    p = uip_buf;
    for (uint16_t i = 0; i < rpv.received_packet_size; i++)
        *p++ = read_buffer_memory();

    uip_len = rpv.received_packet_size;

#   ifdef OPENVPN_SUPPORT
    uip_stack_set_active(STACK_OPENVPN);
#   else
    uip_stack_set_active(STACK_MAIN);
#   endif

    /* process packet */
    struct uip_eth_hdr *packet = (struct uip_eth_hdr *)&uip_buf;
    switch (ntohs(packet->type)) {

#       if !UIP_CONF_IPV6
        /* process arp packet */
        case UIP_ETHTYPE_ARP:
#           ifdef DEBUG_NET
            debug_printf("net: arp packet received\n");
#           endif
            uip_arp_arpin();

            /* if there is a packet to send, send it now */
            if (uip_len > 0)
                transmit_packet();

            break;
#       endif /* !UIP_CONF_IPV6 */

#       if UIP_CONF_IPV6
        /* process ip packet */
        case UIP_ETHTYPE_IP6:
#           ifdef DEBUG_NET
            uart_puts_P("net: ip6 packet received\r\n");
#           endif
#       else /* !UIP_CONF_IPV6 */
        /* process ip packet */
        case UIP_ETHTYPE_IP:
#           ifdef DEBUG_NET
            debug_printf("net: ip packet received\n");
#           endif
            uip_arp_ipin();
#       endif /* !UIP_CONF_IPV6 */

            uip_input();

            /* if there is a packet to send, send it now */
            if (uip_len > 0) {

                /* check if an arp request has to be send */
#               if UIP_CONF_IPV6
            	uip_neighbor_out();
#               else
                uip_arp_out();
#		endif

                transmit_packet();
            }

            break;
#       ifdef DEBUG_UNKNOWN_PACKETS
        default:
            /* debug output */
            debug_printf("net: unknown packet, %02x%02x%02x%02x%02x%02x "
                         "-> %02x%02x%02x%02x%02x%02x, type 0x%02x%02x\n",
                         packet->src.addr[0],
                         packet->src.addr[1],
                         packet->src.addr[2],
                         packet->src.addr[3],
                         packet->src.addr[4],
                         packet->src.addr[5],
                         packet->dest.addr[0],
                         packet->dest.addr[1],
                         packet->dest.addr[2],
                         packet->dest.addr[3],
                         packet->dest.addr[4],
                         packet->dest.addr[5],
                         HI8(ntohs(packet->type)),
                         LO8(ntohs(packet->type)));
            break;
#       endif
    }

    /* advance receive read pointer, ensuring that an odd value is programmed
     * (next_receive_packet_pointer is always even), see errata #13 */
    if ( (enc28j60_next_packet_pointer - 1) < RXBUFFER_START
            || (enc28j60_next_packet_pointer - 1) > RXBUFFER_END) {

        write_control_register(REG_ERXRDPTL, LO8(RXBUFFER_END));
        write_control_register(REG_ERXRDPTH, HI8(RXBUFFER_END));

    } else {

        write_control_register(REG_ERXRDPTL, LO8(enc28j60_next_packet_pointer - 1));
        write_control_register(REG_ERXRDPTH, HI8(enc28j60_next_packet_pointer - 1));

    }

    /* decrement packet counter */
    bit_field_set(REG_ECON2, _BV(PKTDEC));

} /* }}} */
#endif


#ifdef ENC28J60_SUPPORT
void transmit_packet(void)
/* {{{ */ {

    /* wait for any transmits to end, with timeout */
    uint8_t timeout = 100;
    while (read_control_register(REG_ECON1) & _BV(ECON1_TXRTS) && timeout-- > 0);

    if (timeout == 0) {
        debug_printf("net: timeout waiting for TXRTS, aborting transmit!\n");
        return;
    }

    uint16_t start_pointer = TXBUFFER_START;

    /* set send control registers */
    write_control_register(REG_ETXSTL, LO8(start_pointer));
    write_control_register(REG_ETXSTH, HI8(start_pointer));

    write_control_register(REG_ETXNDL, LO8(start_pointer + uip_len));
    write_control_register(REG_ETXNDH, HI8(start_pointer + uip_len));

    /* set pointer to beginning of tx buffer */
    set_write_buffer_pointer(start_pointer);

    /* write override byte */
    write_buffer_memory(0);

    /* write data */
    for (uint16_t i = 0; i < uip_len; i++)
        write_buffer_memory(uip_buf[i]);

#   ifdef ENC28J60_REV4_WORKAROUND
    /* reset transmit hardware, see errata #12 */
    bit_field_set(REG_ECON1, _BV(ECON1_TXRST));
    bit_field_clear(REG_ECON1, _BV(ECON1_TXRST));
#   endif

    /* transmit packet */
    bit_field_set(REG_ECON1, _BV(ECON1_TXRTS));

} /* }}} */
#endif
