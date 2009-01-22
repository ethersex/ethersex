/**********************************************************
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * @author      Gregor B.
 * @author      Dirk Pannenbecker
 * @date        22.01.2009

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

#include <stdlib.h>

#include "../config.h"

#ifdef RF12_POWERSWITCH_SUPPORT

#include "rfm12.h"
#include "rf12_powerswitch.h"

const uint8_t dosencode[]          = {6,12,6,6,12,12,6,12, 6,6,12,6,12,6,12,6, 12,6,12,6,12,6,12,6,12};
const uint8_t dosencode_1_on[]     = {6,12,6,12,6,12,12,6,6,12,6,12,6,12,12,6};
const uint8_t dosencode_1_off[]    = {6,12,6,12,6,12,6,12,6,12,6,12,6,12,6,12};
const uint8_t dosencode_2_on[]     = {12,6,6,12,6,12,12,6,6,12,6,12,12,6,12,6};
const uint8_t dosencode_2_off[]    = {12,6,6,12,6,12,6,12,6,12,6,12,12,6,6,12};
const uint8_t dosencode_3_on[]     = {12,6,12,6,6,12,12,6,6,12,6,12,12,6,6,12};
const uint8_t dosencode_3_off[]    = {12,6,12,6,6,12,6,12,6,12,6,12,12,6,12,6};
const uint8_t dosencode_3_up[]     = {12,6,12,6,6,12,6,12,12,6,6,12,6,12,12,6};
const uint8_t dosencode_3_down[]   = {12,6,12,6,6,12,12,6,12,6,6,12,6,12,6,12};
const uint8_t dosencode_4_on[]     = {6,12,12,6,6,12,12,6,6,12,6,12,6,12,6,12};
const uint8_t dosencode_4_off[]    = {6,12,12,6,6,12,6,12,6,12,6,12,6,12,12,6};
const uint8_t dosencode_4_up[]     = {6,12,12,6,6,12,6,12,12,6,6,12,12,6,12,6};
const uint8_t dosencode_4_down[]   = {6,12,12,6,6,12,12,6,12,6,6,12,12,6,6,12};
const uint8_t dosencode_all_on[]   = {12,6,12,6,12,6,12,6,6,12,6,12,6,12,6,12};
const uint8_t dosencode_all_off[]  = {12,6,12,6,12,6,6,12,6,12,6,12,6,12,12,6};
const uint8_t dosencode_34_up[]    = {12,6,12,6,12,6,6,12,12,6,6,12,12,6,12,6};
const uint8_t dosencode_34_down[]  = {12,6,12,6,12,6,12,6,12,6,6,12,12,6,6,12};
const uint8_t *dosencodes[]        = { dosencode_1_on, dosencode_1_off, dosencode_2_on, dosencode_2_off, dosencode_3_on, dosencode_3_off, dosencode_4_on, dosencode_4_off, dosencode_all_on, dosencode_all_off, dosencode_3_up, dosencode_3_down, dosencode_4_up, dosencode_4_down,dosencode_34_up, dosencode_34_down };

enum dosencommands {
  DOSE_1_ON     = 0,
  DOSE_1_OFF,
  DOSE_2_ON,
  DOSE_2_OFF,
  DOSE_3_ON,
  DOSE_3_OFF,
  DOSE_4_ON,
  DOSE_4_OFF,
  DOSE_ALL_ON,
  DOSE_ALL_OFF,
  DOSE_3_UP,
  DOSE_3_DOWN,
  DOSE_4_UP,
  DOSE_4_DOWN,
  DOSE_34_UP,
  DOSE_34_DOWN,
  DOSE_COM_MAX
};

#ifdef RF12_POWERSWITCH_SENDER_SUPPORT
void rf12_powerswitch_code_send(uint16_t *command, uint8_t cnt) {
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--) {              // Sequenz 4x send
    uint8_t iii=0;
    uint8_t iv=0;
    for(iv=0;iv<25;iv++) {
      rf12_powerswitch_trigger(iii^=1,dosencode[iv]);
    }
    for(iv=0;iv<16;iv++) {
      rf12_powerswitch_trigger(iii^=1,(uint8_t)command[iv]);
    }
    rf12_powerswitch_trigger(0,24);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void rf12_powerswitch_send(uint8_t dosencom, uint8_t cnt) {
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--) {              // Sequenz 4x send
    uint8_t iii=0;
    uint8_t iv=0;
    for(iv=0;iv<25;iv++) {
      rf12_powerswitch_trigger(iii^=1,dosencode[iv]);
    }
    for(iv=0;iv<16;iv++) {
      rf12_powerswitch_trigger(iii^=1,dosencodes[dosencom][iv]);
    }
    rf12_powerswitch_trigger(0,24);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void rf12_powerswitch_trigger(uint8_t level, uint16_t us) {
  if (level) {
    rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3)); // 2. PwrMngt TX on
#ifdef HAVE_RFM12_TX_PIN
    PIN_SET(RFM12_TX_PIN);
#endif
    for(;us>0;us--)
      _delay_us(100);
  }
  else {
    rfm12_trans(0x8208);                      // 2. PwrMngt TX off
#ifdef HAVE_RFM12_TX_PIN
    PIN_CLEAR(RFM12_TX_PIN);
#endif
    for(;us>0;us--)
      _delay_us(100);
  }
}
#endif // RF12_POWERSWITCH_SENDER_SUPPORT
#endif // RF12_POWERSWITCH_SUPPORT