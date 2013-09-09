/*
 * Copyright (c) 2013 by Frank Sautter <ethersix@sautter.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _PPORT_H
#define _PPORT_H


struct pport_buffer
{
  uint16_t len;
  uint16_t sent;
  uint8_t data[PPORT_BUFFER_LEN];
};

void pport_init(void);
uint8_t pport_rxstart(uint8_t * data, uint16_t len);

extern struct pport_buffer pport_recv_buffer;
uint8_t pport_delayed_ack;
#ifdef DEBUG_PPORT
uint32_t pport_rx_bytes;
uint32_t pport_rx_bufferfull;
uint32_t pport_eth_retransmit;
#endif
#if PPORT_FLUSH > 0
extern uint8_t pport_lf;
#endif


#endif /* _PPORT_H */
