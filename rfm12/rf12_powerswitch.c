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

#ifdef RF12_POWERSWITCH_SUPPORT

#include "rfm12.h"
#include "rf12_powerswitch.h"

void rf12_powerswitch_trigger(uint8_t , uint16_t);

#ifdef RF12_POWERSWITCH_SENDER_SUPPORT
void
rf12_powerswitch_encode_byte(uint8_t *code, uint8_t append, uint8_t byte, uint8_t cnt)
{
  for (uint8_t i=0;i<cnt;i++)
  {
    code[append+i]=((byte & (1<<(cnt-i-1)))>0 ?12:6);
  }
}

void
rf12_powerswitch_encode_tribit(uint8_t *code, uint8_t append, uint8_t byte, uint8_t cnt)
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
rf12_powerswitch_tevion_send(uint8_t * housecode, uint8_t * command, uint8_t delay, uint8_t cnt)
{
  uint8_t code[41];

  for(uint8_t i=0;i<3;i++)
  {
    rf12_powerswitch_encode_byte(code, i*8, housecode[i], 8);
  }
  code[24]=((housecode[2] & (1<<0))>0 ?6:12);
  for(uint8_t i=0;i<2;i++)
  {
    rf12_powerswitch_encode_byte(code, (i*8)+25, command[i], 8);
  }
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--)               // Sequenz cnt send
  {
    uint8_t rf12_trigger_level=0;
    for(uint8_t i=0;i<41;i++)
    {
      rf12_powerswitch_trigger(rf12_trigger_level^=1,code[i]*delay);
    }
    rf12_powerswitch_trigger(0,24*delay);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void
rf12_powerswitch_2272_send(uint8_t *command, uint8_t delay, uint8_t cnt)
{
  uint8_t code[49];

  for(uint8_t i=0;i<3;i++)
  {
    rf12_powerswitch_encode_tribit(code, i*16, command[i], 8);
  }
  code[48]=7; //sync
  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t ii=cnt;ii>0;ii--)               // Sequenz cnt send
  {
    uint8_t rf12_trigger_level=0;
    for(uint8_t i=0;i<49;i++)
    {
      rf12_powerswitch_trigger(rf12_trigger_level^=1,code[i]*delay);
    }
    rf12_powerswitch_trigger(0,24*delay);
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}

void
rf12_powerswitch_trigger(uint8_t level, uint16_t us)
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
#endif // RF12_POWERSWITCH_SENDER_SUPPORT

#ifdef RF12_POWERSWITCH_RECEIVER_SUPPORT
void
rf12_powerswitch_receiver_init()
{
  rfm12_prologue ();
  rfm12_trans(0x82C0);                        // 2. PwrMngt TX off, enable whole receiver chain
  rfm12_trans(0xC238);                        // 6. Data Filter Command
  rfm12_epilogue ();
}

void
rf12_powerswitch_receiver_deinit()
{
  rfm12_init();
}
#endif // RF12_POWERSWITCH_RECEIVER_SUPPORT
#endif // RF12_POWERSWITCH_SUPPORT
