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
#include <avr/wdt.h>

#include "config.h"
#include "core/heartbeat.h"

#include "rfm12.h"
#include "rfm12_ask.h"


void rfm12_ask_trigger(uint8_t , uint16_t);

uint8_t ask_2272_1527_pulse_duty_factor[4]={13,5,7,11};

#ifdef RFM12_ASK_SENDER_SUPPORT

void
rfm12_ask_2272_1527_switch(uint8_t ask_type)
{
  if (ask_type == T_2272)
  {
    ask_2272_1527_pulse_duty_factor[0]=13;
    ask_2272_1527_pulse_duty_factor[1]=5;
    ask_2272_1527_pulse_duty_factor[2]=7;
    ask_2272_1527_pulse_duty_factor[3]=11;
  } else if (ask_type == T_1527)
  {
    ask_2272_1527_pulse_duty_factor[0]=9;
    ask_2272_1527_pulse_duty_factor[1]=3;
    ask_2272_1527_pulse_duty_factor[2]=3;
    ask_2272_1527_pulse_duty_factor[3]=9;
  }
}


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
      code[append+(i*2)]=ask_2272_1527_pulse_duty_factor[0];
      code[append+(i*2)+1]=ask_2272_1527_pulse_duty_factor[1];
    } else {
      code[append+(i*2)]=ask_2272_1527_pulse_duty_factor[2];
      code[append+(i*2)+1]=ask_2272_1527_pulse_duty_factor[3];
    }
  }
}

#ifdef RFM12_ASK_TEVION_SUPPORT
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
    wdt_kick();
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
#endif /* RFM12_ASK_TEVION_SUPPORT */

#ifdef RFM12_ASK_INTERTECHNO_SUPPORT
void 
rfm12_ask_intertechno_send_bit(uint8_t bit)
{
   if (bit) {
      rfm12_ask_trigger(1, INTERTECHNO_PERIOD);
      rfm12_ask_trigger(0, 3*INTERTECHNO_PERIOD);
      rfm12_ask_trigger(1, 3*INTERTECHNO_PERIOD);
      rfm12_ask_trigger(0, INTERTECHNO_PERIOD);
   }
   else
   {
      rfm12_ask_trigger(1, INTERTECHNO_PERIOD);
      rfm12_ask_trigger(0, 3*INTERTECHNO_PERIOD);
      rfm12_ask_trigger(1, INTERTECHNO_PERIOD);
      rfm12_ask_trigger(0, 3*INTERTECHNO_PERIOD);
   }
}

void 
rfm12_ask_intertechno_send_sync()
{
   rfm12_ask_trigger(1, INTERTECHNO_PERIOD);
   rfm12_ask_trigger(0, 31*INTERTECHNO_PERIOD);
}

void
rfm12_ask_intertechno_send(uint8_t family, uint8_t group, 
   uint8_t device, uint8_t command)
{
  uint8_t code[12];

  family -= 1;
  group -= 1;
  device -= 1;

  code[0] = 1 & family;
  code[1] = 1 & (family >> 1);
  code[2] = 1 & (family >> 2);
  code[3] = 1 & (family >> 3);

  code[4] = 1 & device;
  code[5] = 1 & (device >> 1);

  code[6] = 1 & group;
  code[7] = 1 & (group >> 1);

  if (command == 0) 
  {
     code[8] = 0;
     code[9] = 1;
     code[10] = 1;
     code[11] = 0;
  }
  else
  {
     code[8] = 0;
     code[9] = 1;
     code[10] = 1;
     code[11] = 1;
  }

  rfm12_prologue ();
  rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3));   // 2. PwrMngt TX on
  for(uint8_t j = 6; j > 0; j--)         // send Sequenz 6 times
  {
    wdt_kick();
    for(uint8_t i = 0; i < 12; i++)
    {
       rfm12_ask_intertechno_send_bit(code[i]);
    }
    rfm12_ask_intertechno_send_sync();
  }
  rfm12_trans(0x8208);                        // 2. PwrMngt TX off
  rfm12_epilogue ();
}
#endif /* RFM12_ASK_INTERTECHNO_SUPPORT */

#if defined RFM12_ASK_2272_SUPPORT || defined RFM12_ASK_1527_SUPPORT
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
    wdt_kick();
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
#endif /* RFM12_ASK_2272_SUPPORT */

void
rfm12_ask_trigger(uint8_t level, uint16_t us)
{
  if (level)
  {
    rfm12_trans(0x8200|(1<<5)|(1<<4)|(1<<3)); // 2. PwrMngt TX on
		#ifdef HAVE_RFM12_TX_PIN
		PIN_SET(RFM12_TX_PIN);
		#endif
		ACTIVITY_LED_RFM12_TX;
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
