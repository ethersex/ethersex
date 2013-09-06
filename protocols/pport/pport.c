/*
 *
 * Copyright (c) 2013 by Frank Sautter <ethersix@sautter.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

/*
 * This module allows to receive parallel/centronics/ieee1284 printer data
 * and forward it via tcp.
 * It needs only a few additional wires on an pollin avr-netio (minimum PD2 or
 * PD3 as /STROBE on pin1 of the 25pin sub-D header).
 * On sending systems that do not pay attention to the /ACK signal an
 * additional 74HCT112 J/K-FlipFlip is needed to assure the tight BUSY signal
 * timing.
 *
 *                       +-------------+
 *   1 : /STROBE  : /C1 -| 1    7    16|- Vcc :  Vcc
 *           Vcc  :  K1 -| 2    4    15|- /R1 :  Vcc
 *           GND  :  J1 -| 3    H    14|- /R2 :  Vcc
 *  10 :    /ACK  : /S1 -| 4    C    13|- /C2 :  Vcc
 *          n.c.  :  Q1 -| 5    T    12|- K2  :  GND
 *  11 :    BUSY  : /Q1 -| 6    1    11|- J2  :  GND
 *          n.c.  : /Q2 -| 7    1    10|- /S2 :  GND
 *           GND  : GND -| 8    2     9|- Q2  :  n.c.
 *                       +-------------+
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"
#include "pport.h"
#include "pport_net.h"

struct pport_buffer pport_recv_buffer;
uint8_t pport_delayed_ack;
#ifdef DEBUG_PPORT
uint32_t pport_rx_bytes;
uint32_t pport_rx_bufferfull;
uint32_t pport_eth_retransmit;
#endif
#if PPORT_FLUSH > 0
uint8_t pport_lf;
#endif



void
pport_init(void)
{
  PPORT_DATA_DDR = 0x00;            // set data as input
  PPORT_DATA_PORT = 0xff;           // enable pullup on data signals
  PIN_SET(PPORT_ACK);
  DDR_CONFIG_OUT(PPORT_ACK);
  PIN_CLEAR(PPORT_PE);
  DDR_CONFIG_OUT(PPORT_PE);
  PIN_SET(PPORT_SEL);
  DDR_CONFIG_OUT(PPORT_SEL);
  DDR_CONFIG_IN(PPORT_STROBE);
  PIN_SET(PPORT_STROBE);            // enable pullup

  // enable interrupt
  _EIMSK |= _BV(PPORT_INT_PIN);
  _EICRA = (_EICRA & ~PPORT_INT_ISCMASK) | PPORT_INT_ISC;

#ifdef PPORT_HAVE_74112
  DDR_CONFIG_IN(PPORT_BUSY);
  PIN_CLEAR(PPORT_ACK);             // reset JK-FF
  PIN_SET(PPORT_ACK);
#else
  DDR_CONFIG_OUT(PPORT_BUSY);
  PIN_CLEAR(PPORT_BUSY);            // we are ready
#endif
}

ISR(PPORT_VECTOR)
{
#ifndef PPORT_HAVE_74112
  PIN_SET(PPORT_BUSY);              // set BUSY to active
#endif
  uint8_t v = PPORT_DATA_PIN;       // read data
  if (pport_recv_buffer.len < PPORT_BUFFER_LEN)
    pport_recv_buffer.data[pport_recv_buffer.len++] = v;
#ifdef DEBUG_PPORT
  else
    pport_rx_bufferfull++;
#endif
#if PPORT_FLUSH > 0
  if (v == 0x0A)
    pport_lf = 1;
#endif

#ifdef DEBUG_PPORT
  pport_rx_bytes++;
#endif

  if (pport_recv_buffer.len < PPORT_BUFFER_LEN - 1)
  {
    PIN_CLEAR(PPORT_ACK);           // send ACK signal
#ifndef PPORT_HAVE_74112
    PIN_CLEAR(PPORT_BUSY);          // set BUSY inactive
#endif
    PIN_SET(PPORT_ACK);
  }
  else
    pport_delayed_ack = 1;          // the buffer is full
}

/*
  -- Ethersex META --
  dnl pport_init call must be done after network_init.
  dnl Therefore we initialize via net_init and control the
  dnl order via the Makefile.

  header(protocols/pport/pport.h)

  net_init(pport_init)
*/
