/**********************************************************
 Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>
 * @author      Benedikt K.
 * @author      Juergen Eckert
 * @author    Ulrich Radig (mail@ulrichradig.de) www.ulrichradig.de 
 * @date        04.06.2007  

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/

#ifndef __RFM12_H
#define __RFM12_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include <util/delay.h>
#include "config.h"


/* Prologue/epilogue macros, disabling/enabling interrupts. 
   Be careful, these are not well suited to be used as if-blocks. */
#define rfm12_prologue()			\
  uint8_t sreg = SREG; cli();
#define rfm12_epilogue()			\
  SREG = sreg;


// initialize module
void rfm12_init(void);

// transfer 1 word to/from module
unsigned short rfm12_trans(unsigned short wert);


#define RxBW400		1
#define RxBW340		2
#define RxBW270		3
#define RxBW200		4
#define RxBW134		5
#define RxBW67		6

#define TxBW15		0
#define TxBW30		1
#define TxBW45		2
#define TxBW60		3
#define TxBW75		4
#define TxBW90		5
#define TxBW105		6
#define TxBW120		7


#define RFM12TxBDW(kfrq)	((uint8_t)(kfrq/15)-1)

/* macro for calculating frequency value out of frequency in MHz */
#define RFM12FREQ(freq)	((freq-430.0)/0.0025)	

// set receiver settings
void rfm12_setbandwidth(uint8_t bandwidth, uint8_t gain, uint8_t drssi);

// set center frequency
void rfm12_setfreq(unsigned short freq);



#ifdef RFM12_IP_SUPPORT

typedef enum {
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
#endif	/* RFM12_SOURCE_ROUTE_ALL */
  RFM12_TX_SIZE_HI,
  RFM12_TX_SIZE_LO,
  RFM12_TX_DATA,
  RFM12_TX_DATAEND,
  RFM12_TX_SUFFIX_1,
  RFM12_TX_SUFFIX_2,
  RFM12_TX_END
} rfm12_status_t;


/* Current RFM12 transceiver status. */
rfm12_status_t rfm12_status;

#define rfm12_tx_active()  (rfm12_status >= RFM12_TX)


#define rfm12_int_enable()			\
  _EIMSK |= _BV(RFM12_INT_PIN);
#define rfm12_int_disable()			\
  _EIMSK &= ~_BV(RFM12_INT_PIN);


#define RFM12_BUFFER_LEN    (UIP_CONF_BUFFER_SIZE - RFM12_BRIDGE_OFFSET)
#define RFM12_DATA_LEN      (RFM12_BUFFER_LEN - RFM12_LLH_LEN)
#define rfm12_buf           (uip_buf + RFM12_BRIDGE_OFFSET)
#define rfm12_data          (rfm12_buf + RFM12_LLH_LEN)


#ifdef TEENSY_SUPPORT
#  if (RFM12_BUFFER_LEN + (defined(RFM12_SOURCE_ROUTE_ALL) ? 3 : 0))  > 254
#    error "modify code or shrink (shared) uIP buffer."
#  endif
typedef uint8_t rfm12_index_t;
#else   /* TEENSY_SUPPORT */
typedef uint16_t rfm12_index_t;
#endif	/* not TEENSY_SUPPORT */

//##############################################################################

#define LNA_0		0
#define LNA_6		1
#define LNA_14		2
#define LNA_20		3

#define RSSI_103	0
#define RSSI_97		1
#define RSSI_91		2
#define RSSI_85		3
#define RSSI_79		4
#define RSSI_73		5
#define RSSI_67		6
#define	RSSI_61		7

#define PWRdB_0		0
#define PWRdB_3		1
#define PWRdB_6		2
#define PWRdB_9		3
#define PWRdB_12	4
#define PWRdB_15	5
#define PWRdB_18	6
#define PWRdB_21	7

//##############################################################################


/* how many calls to wait before a retransmit */
#define RFM12_TXDELAY 0x10


// set baudrate
void rfm12_setbaud(unsigned short baud);

// set transmission settings
void rfm12_setpower(uint8_t power, uint8_t mod);

// start receiving a package
uint8_t rfm12_rxstart(void);

// readout the package, if one arrived
rfm12_index_t rfm12_rxfinish(void);

// start transmitting a package of size size
void rfm12_txstart(rfm12_index_t size);

void rfm12_process (void);


extern uint8_t rfm12_bandwidth;
extern uint8_t rfm12_gain;
extern uint8_t rfm12_drssi;



#else /* not RFM12_IP_SUPPORT */

#define rfm12_int_enable()  do { } while(0)
#define rfm12_int_disable() do { } while(0)
#endif

/* return the current rfm12 status word */
uint16_t rfm12_get_status (void);


#endif //__RFM12_H
