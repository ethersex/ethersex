/**********************************************************
 * Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2007, 2008 Stefan Siegl <stesie@brokenpipe.de>
 *
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

#include <stdlib.h>

#include "../config.h"
#include "../uip/uip.h"
#include "../spi.h"
#include "rfm12.h"
#include "../crypto/encrypt-llh.h"

/* On the bridge-side this is the ID assigned to the beacon (read: bridge),
 * i.e. the ID that's regularly broadcast.  On client side it's the ID of
 * the beacon seen last, i.e. the one that's responsible for transmitting
 * the next packet. */
uint8_t rfm12_beacon_code = CONF_RFM12_BEACON_ID;

enum RFM12_STATUS{
  RFM12_OFF,
  RFM12_RX,
  RFM12_NEW,
  RFM12_TX,
  RFM12_TX_PREAMPLE_1,
  RFM12_TX_PREAMPLE_2,
  RFM12_TX_PREFIX_1,
  RFM12_TX_PREFIX_2,
  RFM12_TX_SIZE,
  RFM12_TX_DATA,
  RFM12_TX_DATAEND,
  RFM12_TX_SUFFIX_1,
  RFM12_TX_SUFFIX_2,
  RFM12_TX_END
};

uint8_t RFM12_akt_status = RFM12_OFF;

uint8_t RFM12_Index = 0;
uint8_t RFM12_Txlen = 0;
unsigned short RFM12_i_status = 0;

#ifndef RFM12_SHARE_UIP_BUF
uint8_t RFM12_Data[RFM12_DataLength];
#endif


SIGNAL(RFM12_INT_SIGNAL)
{
  if(RFM12_akt_status == RFM12_RX)
    {
      if(RFM12_Index < RFM12_DataLength)
	{
	  RFM12_Data[RFM12_Index++] = rfm12_trans(0xB000) & 0x00FF;
#ifdef RFM12_BLINK_PORT
	  RFM12_BLINK_PORT |= RFM12_RX_PIN;
#endif
	}
      else
	{
	  rfm12_trans(0x8208);
          RFM12_akt_status = RFM12_OFF;
	  rfm12_rxstart();
#ifdef RFM12_BLINK_PORT
	  RFM12_BLINK_PORT &= ~RFM12_RX_PIN;
#endif
	}

      if(RFM12_Index >= RFM12_Data[0] + 1)
	{
	  rfm12_trans(0x8208);
	  RFM12_akt_status = RFM12_NEW;	
	}
    }

  else if(RFM12_akt_status >= RFM12_TX)
    {
      if(RFM12_akt_status != RFM12_TX_DATA){
        if(RFM12_akt_status < RFM12_TX_PREFIX_1 || RFM12_akt_status > RFM12_TX_DATA)
          rfm12_trans(0xB8AA);
        else if(RFM12_akt_status == RFM12_TX_PREFIX_1)
          rfm12_trans(0xB82D);
        else if(RFM12_akt_status == RFM12_TX_PREFIX_2)
          rfm12_trans(0xB8D4);
        else if(RFM12_akt_status == RFM12_TX_SIZE)
          rfm12_trans(0xB800 | RFM12_Txlen);
        RFM12_akt_status++;
        if(RFM12_akt_status == RFM12_TX_END){
          RFM12_akt_status = RFM12_OFF;
#ifdef RFM12_BLINK_PORT
          RFM12_BLINK_PORT &= ~RFM12_TX_PIN;
#endif
          rfm12_trans(0x8208);	/* TX off */
          rfm12_rxstart();
        }
      }
      else if(RFM12_akt_status == RFM12_TX_DATA){
        rfm12_trans(0xB800 | RFM12_Data[RFM12_Index++]);
        if(RFM12_Index >= RFM12_Txlen)
          RFM12_akt_status = RFM12_TX_DATAEND;
      }
    }
  else
    {
      RFM12_i_status = rfm12_trans(0x0000);/* dummy read (get Statusregister) */
#ifdef SYSLOG_SUPPORT
      char text[40];
      snprintf(text, 40, "rfm12 interrupt RFM12_i_status: %04X %02X\n", RFM12_i_status, RFM12_akt_status);
      syslog_send(text);
#endif
      /* FIXME what happend */
    }
}


unsigned short
rfm12_trans(unsigned short wert)
{	
  unsigned short werti = 0;
	
  PIN_CLEAR(SPI_CS_RFM12);
	
  /* spi clock down */
  _SPCR0 |= _BV(SPR0);

  werti = (spi_send ((0xFF00 & wert) >> 8) << 8);
  werti += spi_send (0x00ff & wert);

  /* spi clock high */
  _SPCR0 &= ~_BV(SPR0);

  PIN_SET(SPI_CS_RFM12);
  return werti;
}


void
rfm12_init(void)
{
  uint8_t i;

  for (i=0; i<10; i++)
    _delay_ms(10);		/* wait until POR done */

  rfm12_trans(0xC0E0);		/* AVR CLK: 10MHz */
  rfm12_trans(0x80D7);		/* Enable FIFO */
  rfm12_trans(0xC2AB);		/* Data Filter: internal */
  rfm12_trans(0xCA81);		/* Set FIFO mode */
  rfm12_trans(0xE000);		/* disable wakeuptimer */
  rfm12_trans(0xC800);		/* disable low duty cycle */
  rfm12_trans(0xC4F7);		/* AFC settings: autotuning: -10kHz...+7,5kHz */
  rfm12_trans(0x0000);
  
  RFM12_akt_status = RFM12_OFF;
#ifdef RFM12_BLINK_PORT
  RFM12_BLINK_DDR |= RFM12_RX_PIN | RFM12_TX_PIN;
#endif

  _EIMSK |= _BV(RFM12_INT_PIN);
}


/* Prologue/epilogue macros, disabling/enabling interrupts. 
   Be careful, these are not well suited to be used as if-blocks. */
#define rfm12_prologue()			\
  uint8_t sreg = SREG; cli();
#define rfm12_epilogue()			\
  SREG = sreg;


#ifndef TEENSY_SUPPORT
void
rfm12_setbandwidth(uint8_t bandwidth, uint8_t gain, uint8_t drssi)
{
  rfm12_prologue ();
  rfm12_trans (0x9400 | ((bandwidth & 7) << 5)|((gain & 3) << 3) | (drssi & 7));
  rfm12_epilogue ();
}


void
rfm12_setfreq(unsigned short freq)
{	
  if (freq < 96)		/* 430,2400MHz */
    freq = 96;

  else if (freq > 3903)		/* 439,7575MHz */
    freq = 3903;

  rfm12_prologue ();
  rfm12_trans (0xA000 | freq);
  rfm12_epilogue ();
}


void
rfm12_setbaud(unsigned short baud)
{
  if (baud < 663)
    return;

  rfm12_prologue ();

  /* Baudrate = 344827,58621 / (R + 1) / (1 + CS * 7) */
  if (baud < 5400)
    rfm12_trans(0xC680 | ((43104 / baud) - 1));
  else
    rfm12_trans(0xC600 | ((344828UL / baud) - 1));

  rfm12_epilogue ();
}


void
rfm12_setpower(uint8_t power, uint8_t mod)
{	
  rfm12_prologue ();
  rfm12_trans(0x9800|(power&7)|((mod&15)<<4));
  rfm12_epilogue ();
}
#endif


uint8_t
rfm12_rxstart(void)
{
  if(RFM12_akt_status != RFM12_OFF)
    return(1);			/* rfm12 is not free for RX or now in RX */

  rfm12_prologue ();

  rfm12_trans(0x82C8);		/* RX on */
  rfm12_trans(0xCA81);		/* set FIFO mode */
  rfm12_trans(0xCA83);		/* enable FIFO */

  rfm12_epilogue ();

  RFM12_Index = 0;
  RFM12_akt_status = RFM12_RX;

  return(0);
}


uint8_t 
rfm12_rxfinish(uint8_t *data)
{
  if(RFM12_akt_status != RFM12_NEW)
    return (255);		/* no new Packet */

#ifdef RFM12_BLINK_PORT
  RFM12_BLINK_PORT &= ~RFM12_RX_PIN;
#endif

  uint8_t i;
  uint8_t len = RFM12_Data[0];

  /* if RFM12_SHARE_UIP_BUF is set, the following will destroy the
     first byte!  Therefore it is essential to copy the data
     in forward direction below (and not use RFM12_Data afterwards). */
  for(i = 0; i < len; i++)
    data[i] = RFM12_Data[i + 1];

#ifdef SKIPJACK_SUPPORT
  rfm12_decrypt (data, &len);
#endif
  RFM12_akt_status = RFM12_OFF;
#ifdef SKIPJACK_SUPPORT
  if (!len)
    rfm12_rxstart ();		/* rfm12_decrypt destroyed the packet. */
#endif
  
  return(len);			/* receive size */
}


uint8_t 
rfm12_txstart(uint8_t *data, uint8_t size)
{
  uint8_t i, l;

  if(RFM12_akt_status > RFM12_RX || (RFM12_akt_status == RFM12_RX && RFM12_Index > 0))
    return(3);                  /* rx or tx in action oder new packet in buffer*/
  
  if(size > RFM12_DataLength)
    return(4);			/* str to big to transmit */

  RFM12_akt_status = RFM12_TX;

#ifdef RFM12_BLINK_PORT
  RFM12_BLINK_PORT |= RFM12_TX_PIN;
#endif

#ifndef RFM12_SHARE_UIP_BUF
  i = size; while (i --)
              RFM12_Data[i] = data[i];
#endif
  i = RFM12_Index = 0;

#ifdef SKIPJACK_SUPPORT
  rfm12_encrypt (RFM12_Data, &size);

  if (!size){
    RFM12_akt_status = RFM12_OFF;
    return 4;
  }
#endif
  RFM12_Txlen = size;

  rfm12_prologue ();
  rfm12_trans(0x8238);		/* TX on */
  rfm12_epilogue ();

  return(0);
}

