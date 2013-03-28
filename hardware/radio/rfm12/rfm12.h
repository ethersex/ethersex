/*
 * Copyright (c) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 Ulrich Radig <mail@ulrichradig.de>
 * Copyright (c) 2012 Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) Benedikt K.
 * Copyright (c) Juergen Eckert
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef __RFM12_H
#define __RFM12_H

#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>
#include "config.h"
#include "core/debug.h"
#include "rfm12_hw.h"

#ifdef DEBUG
#define RFM12_DEBUG(s, args...) printf_P(PSTR("D: " s "\n"), ## args)
#else
#define RFM12_DEBUG(a...)
#endif

/* Prologue/epilogue macros, disabling/enabling interrupts. */
#define rfm12_prologue(m)  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { \
                             rfm12_modul_set_active(m);
#define rfm12_epilogue()   }

typedef struct
{
  volatile uint8_t *rfm12_port;
  uint8_t rfm12_mask;
  uint8_t rfm12_bandwidth;
  uint8_t rfm12_gain;
  uint8_t rfm12_drssi;
} rfm12_modul_t;

enum
{
#ifdef RFM12_IP_SUPPORT
  RFM12_MODULE_IP,
#endif
#ifdef RFM12_ASK_433_SUPPORT
  RFM12_MODULE_ASK,
#endif
#ifdef RFM12_ASK_FS20_SUPPORT
  RFM12_MODULE_FS20,
#endif
  RFM12_MODULE_COUNT
};

extern rfm12_modul_t rfm12_moduls[RFM12_MODULE_COUNT];
extern rfm12_modul_t *rfm12_modul;

#define rfm12_modul_get_active()  (rfm12_modul - rfm12_moduls)
#define rfm12_modul_set_active(i) (rfm12_modul = &rfm12_moduls[i])

uint16_t rfm12_trans(uint16_t);

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

#define RFM12TxBDW(kfrq) ((uint8_t)(kfrq/15)-1)

/* macro for calculating frequency value out of frequency in kHz */
#define RFM12FREQ(freq)	(((freq<800000?freq*2:freq)-860000)/5)
#define RFM12BAND(freq)	(freq<800000?0x80D7:0x80E7)

#define LNA_0           0
#define LNA_6           1
#define LNA_14          2
#define LNA_20          3

#define RSSI_103        0
#define RSSI_97         1
#define RSSI_91         2
#define RSSI_85         3
#define RSSI_79         4
#define RSSI_73         5
#define RSSI_67         6
#define RSSI_61         7

#define PWRdB_0         0
#define PWRdB_3         1
#define PWRdB_6         2
#define PWRdB_9         3
#define PWRdB_12        4
#define PWRdB_15        5
#define PWRdB_18        6
#define PWRdB_21        7


#ifndef TEENSY_SUPPORT
uint16_t rfm12_setbandwidth(uint8_t, uint8_t, uint8_t);
uint16_t rfm12_setfreq(uint16_t);
uint16_t rfm12_setbaud(uint16_t);
uint16_t rfm12_setpower(uint8_t, uint8_t);
#endif
uint16_t rfm12_get_status(void);

#endif /* __RFM12_H */
