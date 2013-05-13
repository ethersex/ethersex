/*
 *
 *          enc28j60 api
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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


#ifndef _ENC28J60_H
#define _ENC28J60_H

#include <avr/io.h>
#include "config.h"

#ifdef ENC28J60_SUPPORT

#define noinline __attribute__((noinline))

/* spi commands */
#define CMD_RCR    0x00 /* address */
#define CMD_RBM    0x3A /* no argument */
#define CMD_WCR    0x40 /* address, data */
#define CMD_WBM    0x7A /* data */
#define CMD_BFS    0x80 /* address, data */
#define CMD_BFC    0xA0 /* address, data */
#define CMD_RESET  0xFF

/* registers */

/* a register consists of 8 bits:
 * bit:     7 6 5 4 3 2 1 0
 * value:   D B B A A A A A
 *
 * meaning: D: if set, register needs one dummy byte when reading
 *          B: bank address
 *          A: register address in bank
 */

/* bank 0 */
#define   REG_ERDPTL    0x00
#define   REG_ERDPTH    0x01
#define   REG_EWRPTL    0x02
#define   REG_EWRPTH    0x03
#define   REG_ETXSTL    0x04
#define   REG_ETXSTH    0x05
#define   REG_ETXNDL    0x06
#define   REG_ETXNDH    0x07
#define   REG_ERXSTL    0x08
#define   REG_ERXSTH    0x09
#define   REG_ERXNDL    0x0A
#define   REG_ERXNDH    0x0B
#define   REG_ERXRDPTL  0x0C
#define   REG_ERXRDPTH  0x0D
#define   REG_ERXWRPTL  0x0E
#define   REG_ERXWRPTH  0x0F
#define   REG_EDMASTL   0x10
#define   REG_EDMASTH   0x11
#define   REG_EDMANDL   0x12
#define   REG_EDMANDH   0x13
#define   REG_EDMADSTL  0x14
#define   REG_EDMADSTH  0x15
#define   REG_EDMACSL   0x16
#define   REG_EDMACSH   0x17
/* bank 1 */
#define   REG_EHT0      (0x00|0x20)
#define   REG_EHT1      (0x01|0x20)
#define   REG_EHT2      (0x02|0x20)
#define   REG_EHT3      (0x03|0x20)
#define   REG_EHT4      (0x04|0x20)
#define   REG_EHT5      (0x05|0x20)
#define   REG_EHT6      (0x06|0x20)
#define   REG_EHT7      (0x07|0x20)
#define   REG_EPMM0     (0x08|0x20)
#define   REG_EPMM1     (0x09|0x20)
#define   REG_EPMM2     (0x0A|0x20)
#define   REG_EPMM3     (0x0B|0x20)
#define   REG_EPMM4     (0x0C|0x20)
#define   REG_EPMM5     (0x0D|0x20)
#define   REG_EPMM6     (0x0E|0x20)
#define   REG_EPMM7     (0x0F|0x20)
#define   REG_EPMCSL    (0x10|0x20)
#define   REG_EPMCSH    (0x11|0x20)
#define   REG_EPMOL     (0x14|0x20)
#define   REG_EPMOH     (0x15|0x20)
#define   REG_EWOLIE    (0x16|0x20)
#define   REG_EWOLIR    (0x17|0x20)
#define   REG_ERXFCON   (0x18|0x20)
#define     BCEN        0
#define     MCEN        1
#define     HTEN        2
#define     MPEN        3
#define     PMEN        4
#define     CRCEN       5
#define     ANDOR       6
#define     UCEN        7
#define   REG_EPKTCNT   (0x19|0x20)
/* bank 2 */
#define   REG_MACON1    (0x00|0x40|0x80)
#define     MARXEN      0
#define     PASSALL     1
#define     RXPAUS      2
#define     TXPAUS      3
#define     LOOPBK      4
#define   REG_MACON2    (0x01|0x40|0x80)
#define     TFUNRST     0
#define     MATXRST     1
#define     RFUNRST     2
#define     MARXRST     3
#define     RNDRST      6
#define     MARST       7
#define   REG_MACON3    (0x02|0x40|0x80)
#define     FULDPX      0
#define     FRMLNEN     1
#define     HFRMEN      2
#define     PHDRLEN     3
#define     TXCRCEN     4
#define     PADCFG0     5
#define     PADCFG1     6
#define     PADCFG2     7
#define   REG_MACON4    (0x03|0x40|0x80)
#define     PUREPRE     0
#define     LONGPRE     1
#define     NOBKOFF     4
#define     BPEN        5
#define     DEFER       6
#define   REG_MABBIPG   (0x04|0x40|0x80)
#define   REG_MAIPGL    (0x06|0x40|0x80)
#define   REG_MAIPGH    (0x07|0x40|0x80)
#define   REG_MACLCON1  (0x08|0x40|0x80)
#define   REG_MACLCON2  (0x09|0x40|0x80)
#define   REG_MAMXFLL   (0x0A|0x40|0x80)
#define   REG_MAMXFLH   (0x0B|0x40|0x80)
#define   REG_MAPHSUP   (0x0D|0x40|0x80)
#define   REG_MICON     (0x11|0x40|0x80)
#define     RSTMII      7
#define   REG_MICMD     (0x12|0x40|0x80)
#define     MIIRD       0
#define     MIISCAN     1
#define   REG_MIREGADR  (0x14|0x40|0x80)
#define   REG_MIWRL     (0x16|0x40|0x80)
#define   REG_MIWRH     (0x17|0x40|0x80)
#define   REG_MIRDL     (0x18|0x40|0x80)
#define   REG_MIRDH     (0x19|0x40|0x80)
/* bank 3 */
#define   REG_MAADR1    (0x00|0x60|0x80)
#define   REG_MAADR0    (0x01|0x60|0x80)
#define   REG_MAADR3    (0x02|0x60|0x80)
#define   REG_MAADR2    (0x03|0x60|0x80)
#define   REG_MAADR5    (0x04|0x60|0x80)
#define   REG_MAADR4    (0x05|0x60|0x80)
#define   REG_EBSTSD    (0x06|0x60)
#define   REG_EBSTCON   (0x07|0x60)
#define   REG_EBSTCSL   (0x08|0x60)
#define   REG_EBSTCSH   (0x09|0x60)
#define   REG_MISTAT    (0x0A|0x60|0x80)
#define     BUSY        0
#define     SCAN        1
#define     NVALID      2
#define   REG_EREVID    (0x12|0x60)
#define   REG_ECOCON    (0x15|0x60)
#define   REG_EFLOCON   (0x17|0x60)
#define   REG_EPAUSL    (0x18|0x60)
#define   REG_EPAUSH    (0x19|0x60)
/* common */
#define   REG_EIE       0x1B
#define     RXERIE  0
#define     TXERIE  1
#define     WOLIE   2
#define     TXIE    3
#define     LINKIE  4
#define     DMAIE   5
#define     PKTIE   6
#define     INTIE   7
#define   REG_EIR       0x1C
#define     RXERIF  0
#define     TXERIF  1
#define     WOLIF   2
#define     TXIF    3
#define     LINKIF  4
#define     DMAIF   5
#define     PKTIF   6
#define   REG_ESTAT     0x1D
#define     CLKRDY   0
#define     TXABRT   1
#define     RXBUSY   2
#define     LATECOL  3
#define     INT      7
#define   REG_ECON2     0x1E
#define     VRPS    3
#define     PWRSV   5
#define     PKTDEC  6
#define     AUTOINC 7
#define   REG_ECON1     0x1F
#define     ECON1_BSEL0   0
#define     ECON1_BSEL1   1
#define     ECON1_RXEN    2
#define     ECON1_TXRTS   3
#define     ECON1_CSUMEN  4
#define     ECON1_DMAST   5
#define     ECON1_RXRST   6
#define     ECON1_TXRST   7
#define         BANK0 0
#define         BANK1 _BV(ECON1_BSEL0)
#define         BANK2 _BV(ECON1_BSEL1)
#define         BANK3 (_BV(ECON1_BSEL0) | _BV(ECON1_BSEL1))

/* PHY */
#define PHY_PHCON1  0x00
#define     PDPXMD      8
#define     PPWRSV      11
#define     PLOOKPDK    14
#define     PRST        15
#define PHY_PHSTAT1 0x01
#define     JBRSTAT     1
#define     LLSTAT      2
#define     PHDPX       11
#define     PFDPX       12
#define PHY_PHHID1  0x02
#define     PLRITY      4
#define     DPXSTAT     9
#define     LSTAT       10
#define     COLSTAT     11
#define     RXSTAT      12
#define     TXSTAT      13
#define PHY_PHHID2  0x03
#define PHY_PHCON2  0x10
#define     HDLDIS  8
#define     JABBER  10
#define     TXDIS   13
#define     FRCLNK  14
#define PHY_PHSTAT2 0x11
#define PHY_PHIE    0x12
#define     PGEIE   1
#define     PLINKIE 4
#define PHY_PHIR    0x13
#define PHY_PHLCON  0x14
#define     STRCH   1
#define     LFRQ0   2
#define     LFRQ1   3
#define     LBCFG0  4
#define     LBCFG1  5
#define     LBCFG2  6
#define     LBCFG3  7
#define     LACFG0  8
#define     LACFG1  9
#define     LACFG2  10
#define     LACFG3  11

/* */

/* constants for sending */
#define POVERRIDE 0
#define PCRCEN 1
#define PPADEN 2
#define PHUGEEN 3

/* masks */
#define REGISTER_ADDRESS_MASK   0x1F
#define REGISTER_BANK_MASK      0x60
#define BANK_MASK               0x03

/* key registers allows access without switching banks */
#define KEY_REGISTERS  0x1B         /* (0x1B to 0x1F)
                                       EIE, EIR, ESTAT, ECON2 and ECON1 */

/* defines */
#define RXBUFFER_START 0x0000   /* start receive buffer at the beginning */
#define RXBUFFER_END   0x0FFF   /* end receive buffer at 4kb */
#define TXBUFFER_START 0x1000   /* start transmit buffer at 4kb */

#define RECEIVE_BUFFER_WRAP(x) ((x) & (RXBUFFER_END))

/* ECOCON setup */
#define ECOCON_UNSET _unset_
#define ECOCON_6 6
#define ECOCON_4 4
#define ECOCON_3 3
#define ECOCON_2 2
#define ECOCON_1 1
#define ECOCON_0 0

/* global variables */
extern int16_t enc28j60_next_packet_pointer;

/* do not do timeout while waiting for spi transfer completed */
/* #define SPI_TIMEOUT */

/* structs */
struct receive_packet_vector_t {
    uint16_t received_packet_size;
    uint8_t state[2];
};

struct transmit_packet_vector_t {
    uint8_t byte[7];
};

#define bit_field_clear(addr,mask) bit_field_modify(addr, mask, CMD_BFC);
#define bit_field_set(addr,mask)   bit_field_modify(addr, mask, CMD_BFS);

/* prototypes */
uint8_t noinline read_control_register(uint8_t address);
uint8_t noinline read_buffer_memory(void);
void noinline write_control_register(uint8_t address, uint8_t data);
void noinline write_buffer_memory(uint8_t data);
void noinline bit_field_modify(uint8_t address, uint8_t mask, uint8_t opcode);
void noinline set_read_buffer_pointer(uint16_t address);
uint16_t noinline get_read_buffer_pointer(void);
void noinline set_write_buffer_pointer(uint16_t address);
uint16_t noinline get_write_buffer_pointer(void);
uint16_t noinline read_phy(uint8_t address);
void noinline write_phy(uint8_t address, uint16_t data);
void noinline reset_controller(void);
void noinline reset_rx(void);
void init_enc28j60(void);
void enc28j60_periodic(void);
void noinline switch_bank(uint8_t bank);
void network_config_load(void);
#if defined(IPV6_SUPPORT) && !defined(IPV6_STATIC_SUPPORT)
void ethernet_config_periodic(void);
#endif

#ifdef DEBUG_ENC28J60
void dump_debug_registers(void);
#else
#define dump_debug_registers(x)
#endif

#endif  /* ENC28J60_SUPPORT */
#endif /* _ENC28J60_H */
