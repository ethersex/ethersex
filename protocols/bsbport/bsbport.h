/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) 2013 by Daniel Lindner <daniel.lindner@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#ifndef _BSBPORT_H
#define _BSBPORT_H

#define SOT_BYTE 0xDC

struct bsbport_msg
{
  union {
    uint32_t raw;
    struct { uint8_t p1, p2, p3, p4; } data;
  } p;
  uint8_t src;
  uint8_t dest;
  uint8_t type;
  int16_t value;
  uint8_t data_length;
  uint8_t data[BSBPORT_MESSAGE_MAX_LEN - 11];
#ifdef BSBPORT_MQTT_SUPPORT
  uint8_t mqtt_new;
#endif
};

struct bsbport_buffer_msg
{
  uint16_t act;
  struct bsbport_msg msg[BSBPORT_MESSAGE_BUFFER_LEN];
};

struct bsbport_buffer_net
{
  uint16_t len;
  uint16_t sent;
  uint8_t data[BSBPORT_BUFFER_LEN];
};

struct bsbport_buffer_rx
{
  uint16_t len;
  uint16_t read;
  uint8_t data[BSBPORT_BUFFER_LEN];
};

enum msg_pos
{
  SOT = 0,
  SRC,
  DEST,
  LEN,
  TYPE,
  P1,
  P2,
  P3,
  P4,
  DATA
};

enum msg_types
{
  INFO = 2,
  SET = 3,
  ACK = 4,
  QUERY = 6,
  ANSWER = 7
};

enum msg_src
{
  SSR = 0,
  RGT1 = 6,
  RGT2 = 7,
  HK = 10,
  All = 15
};

void bsbport_init(void);
uint8_t bsbport_txstart(const uint8_t * const data, const uint16_t len);

extern struct bsbport_buffer_net bsbport_recvnet_buffer;
extern struct bsbport_buffer_net bsbport_send_buffer;
extern struct bsbport_buffer_rx bsbport_recv_buffer;

extern struct bsbport_buffer_msg bsbport_msg_buffer;

extern uint16_t bsbport_rx_ok;
extern uint16_t bsbport_rx_crcerror;
extern uint16_t bsbport_rx_lenghtunder;
extern uint16_t bsbport_rx_lenghtover;
extern uint16_t bsbport_rx_frameerror;
extern uint16_t bsbport_rx_overflow;
extern uint16_t bsbport_rx_parityerror;
extern uint16_t bsbport_rx_bufferfull;
extern uint16_t bsbport_rx_dropped;
extern uint16_t bsbport_rx_net_bufferfull;
extern uint16_t bsbport_eth_retransmit;

#if BSBPORT_FLUSH > 0
extern uint8_t bsbport_lf;
#endif

/* init the usart module ( 8O1 )*/
#define generate_bsbport_usart_init_8O1() \
static void \
usart_init(void) \
{\
  /* The ATmega644 datasheet suggests to clear the global \
   * interrupt flags on initialization ... */\
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) \
  { \
    usart(UBRR,H) = UBRRH_VALUE; \
    usart(UBRR,L) = UBRRL_VALUE; \
    /* set mode 8O1: 8 bits, 1 stop, odd parity, asynchronous usart \
     * and set URSEL, if present, */ \
    usart(UCSR,C) = _BV(usart(UPM,0)) | _BV(usart(UPM,1)) | _BV(usart(UCSZ,0)) | _BV(usart(UCSZ,1)) | _BV_URSEL; \
    /* Enable the RX interrupt and receiver and transmitter */ \
    usart(UCSR,B) |= _BV(usart(TXEN)) | _BV(usart(RXEN)) | _BV(usart(RXCIE)); \
    /* Set or not set the 2x mode */ \
    USART_2X(); \
  } \
}

#endif /* _BSBPORT_H */
