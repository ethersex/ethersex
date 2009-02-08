/**********************************************************
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * @author      Gregor B.
 * @author      Dirk Pannenbecker
 * @author      Guido Pannenbecker
 * @author      Stefan Riepenhausen
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

#ifdef RFM12_ASK_SUPPORT

#include "rfm12.h"
#include "rfm12_ask.h"

void rfm12_ask_trigger(uint8_t , uint16_t);

#ifdef RFM12_ASK_SENDER_SUPPORT
void
rfm12_ask_encode_byte(uint8_t *code, uint8_t append, uint8_t byte, uint8_t cnt)
{
  for (uint8_t i=0;i<cnt;i++)
  {
    code[append+i]=((byte & (1<<(cnt-i-1)))>0 ?12:6);
  }
}

void
rfm12_ask_encode_tribit(uint8_t *code, uint8_t append, uint8_t byte, uint8_t cnt)
{
  for (uint8_t i=0;i<cnt;i++)
  {
    if (byte & (1<<(cnt-i-1))) {
      code[append+(i*2)]=13;
      code[append+(i*2)+1]=5;
    } else {
      code[append+(i*2)]=7;
      code[append+(i*2)+1]=11;
    }
  }
}

void
rfm12_ask_tevion_send(uint8_t * housecode, uint8_t * command, uint8_t delay, uint8_t cnt)
{
  uint8_t code[41];

  for(uint8_t i=0;i<3;i++)
  {
    rfm12_ask_encode_byte(code, i*8, housecode[i], 8);
  }
  code[24]=((housecode[2] & (1<<0))>0 ?6:12);
  for(uint8_t i=0;i<2;i++)
  {
    rfm12_ask_encode_byte(code, (i*8)+25, command[i], 8);
  }
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--)               // Sequenz cnt send
  {
    uint8_t rfm12_trigger_level=0;
    for(uint8_t i=0;i<41;i++)
    {
      rfm12_ask_trigger(rfm12_trigger_level^=1,code[i]*delay);
    }
    rfm12_ask_trigger(0,24*delay);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void
rfm12_ask_2272_send(uint8_t *command, uint8_t delay, uint8_t cnt)
{
  uint8_t code[49];

  for(uint8_t i=0;i<3;i++)
  {
    rfm12_ask_encode_tribit(code, i*16, command[i], 8);
  }
  code[48]=7; //sync
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--)               // Sequenz cnt send
  {
    uint8_t rfm12_trigger_level=0;
    for(uint8_t i=0;i<49;i++)
    {
      rfm12_ask_trigger(rfm12_trigger_level^=1,code[i]*delay);
    }
    rfm12_ask_trigger(0,24*delay);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void
rfm12_ask_trigger(uint8_t level, uint16_t us)
{
  if (level)
  {
    rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3)); // 2. PwrMngt TX on
#ifdef HAVE_RFM12_TX_PIN
    PIN_SET(RFM12_TX_PIN);
#endif
    for(;us>0;us--)
      _delay_us(1);
  }
  else
  {
    rfm12_trans(0x8208);                      // 2. PwrMngt TX off
#ifdef HAVE_RFM12_TX_PIN
    PIN_CLEAR(RFM12_TX_PIN);
#endif
    for(;us>0;us--)
      _delay_us(1);
  }
}
#endif // RFM12_ASK_SENDER_SUPPORT

#ifdef RFM12_ASK_EXTERNAL_FILTER_SUPPORT
void
rfm12_ask_external_filter_init()
{
  rfm12_prologue ();
  rfm12_trans(0x82C0);                        // 2. PwrMngt TX off, enable whole receiver chain
  rfm12_trans(0xC238);                        // 6. Data Filter Command
  rfm12_epilogue ();
}

void
rfm12_ask_external_filter_deinit()
{
  rfm12_init();
}
#endif // RFM12_ASK_EXTERNAL_FILTER_SUPPORT
#endif // RFM12_ASK_SUPPORT
