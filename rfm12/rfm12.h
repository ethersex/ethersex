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
#include "../config.h"

//##############################################################################
/* config */

/* RFM12 Buffer length (max length 254) */
#define RFM12_DataLength	196

#ifndef ENC28J60_SUPPORT
#  define RFM12_SHARE_UIP_BUF
#  undef RFM12_DataLength
#  define RFM12_DataLength (uint8_t)(UIP_CONF_BUFFER_SIZE)
#  define RFM12_Data uip_buf
#endif /* no ENC28J60_SUPPORT */

//##############################################################################

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

#define RFM12TxBDW(kfrq)	((uint8_t)(kfrq/15)-1)

/* macro for calculating frequency value out of frequency in MHz */
#define RFM12FREQ(freq)	((freq-430.0)/0.0025)	

/* how many calls to wait before a retransmit */
#define RFM12_TXDELAY 0x10


// initialize module
void rfm12_init(void);

// transfer 1 word to/from module
unsigned short rfm12_trans(unsigned short wert);

// set center frequency
void rfm12_setfreq(unsigned short freq);

// set baudrate
void rfm12_setbaud(unsigned short baud);

// set transmission settings
void rfm12_setpower(uint8_t power, uint8_t mod);

// set receiver settings
void rfm12_setbandwidth(uint8_t bandwidth, uint8_t gain, uint8_t drssi);

// start receiving a package
uint8_t rfm12_rxstart(void);

// readout the package, if one arrived
uint8_t rfm12_rxfinish(uint8_t *data);

// start transmitting a package of size size
uint8_t rfm12_txstart(uint8_t *data, uint8_t size);

// check whether the package is already transmitted
uint8_t rfm12_txfinished(void);
uint8_t rfm12_Index(void);

// stop all Rx and Tx operations
void rfm12_allstop(void);

void rfm12_process (void);

#ifdef RFM12_BEACON_SUPPORT
extern uint8_t rfm12_beacon_code;
#endif

/* return the current rfm12 status word */
uint16_t rfm12_get_status (void);

#endif //__RFM12_H
