/*
 * Copyright (c) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 Ulrich Radig <mail@ulrichradig.de>
 * Copyright (c) 2012 Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) Benedikt K.
 * Copyright (c) Juergen Eckert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#ifndef __RFM12_NET_H
#define __RFM12_NET_H

#ifdef RFM12_IP_SUPPORT

void rfm12_net_init(void);

typedef enum
{
  RFM12_OFF,
  RFM12_RX,
  RFM12_NEW,
  RFM12_TX,
  RFM12_TX_PREAMBLE_1,
  RFM12_TX_PREAMBLE_2,
  RFM12_TX_PREFIX_1,
  RFM12_TX_PREFIX_2,
#ifdef RFM12_SOURCE_ROUTE_ALL
  RFM12_TX_SRCRT_SZHI,
  RFM12_TX_SRCRT_SZLO,
  RFM12_TX_SRCRT_DEST,
#endif                          /* RFM12_SOURCE_ROUTE_ALL */
  RFM12_TX_SIZE_HI,
  RFM12_TX_SIZE_LO,
  RFM12_TX_DATA,
  RFM12_TX_DATAEND,
  RFM12_TX_SUFFIX_1,
  RFM12_TX_SUFFIX_2,
  RFM12_TX_END
} rfm12_status_t;


/* Current RFM12 transceiver status. */
extern rfm12_status_t rfm12_status;

#define rfm12_tx_active()  (rfm12_status >= RFM12_TX)

#ifdef RFM12_USE_POLL
#define rfm12_int_enable()  do { } while(0)
#define rfm12_int_disable() do { } while(0)
#elif !defined(HAVE_RFM12_PCINT)
#define rfm12_int_enable()			\
	  _EIMSK |= _BV(RFM12_INT_PIN);
#define rfm12_int_disable()			\
	  _EIMSK &= ~_BV(RFM12_INT_PIN);
#endif /* not HAVE_RFM12_PCINT */


#define RFM12_BUFFER_LEN    (UIP_CONF_BUFFER_SIZE - RFM12_BRIDGE_OFFSET)
#define RFM12_DATA_LEN      (RFM12_BUFFER_LEN - RFM12_LLH_LEN)
#define rfm12_buf           (uip_buf + RFM12_BRIDGE_OFFSET)
#define rfm12_data          (rfm12_buf + RFM12_LLH_LEN)


#ifdef TEENSY_SUPPORT
#if (RFM12_BUFFER_LEN + (defined(RFM12_SOURCE_ROUTE_ALL) ? 3 : 0))  > 254
#error "modify code or shrink (shared) uIP buffer."
#endif
typedef uint8_t rfm12_index_t;
#else /* TEENSY_SUPPORT */
typedef uint16_t rfm12_index_t;
#endif /* not TEENSY_SUPPORT */

/* how many calls to wait before a retransmit */
#define RFM12_TXDELAY 0x10


uint8_t rfm12_rxstart(void);
rfm12_index_t rfm12_rxfinish(void);
void rfm12_txstart(rfm12_index_t);
void rfm12_process(void);


#else /* not RFM12_IP_SUPPORT */

#define rfm12_int_enable()  do { } while(0)
#define rfm12_int_disable() do { } while(0)
#endif

#if !defined(RFM12_USE_POLL) || !defined(RFM12_IP_SUPPORT)
#define rfm12_int_process()  do { } while(0)
#endif

#endif /* __RFM12_NET_H */
