/**********************************************************
 * Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright(C) 2007 Stefan Siegl <stesie@brokenpipe.de>
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

struct RFM12_stati
{
  uint8_t Rx:1;
  uint8_t Ack:1;
  uint8_t Tx:1;
  uint8_t Txok:1;
  uint8_t New:1;
};

struct RFM12_stati RFM12_status;
volatile uint8_t RFM12_Index = 0;

#ifndef ENC28J60_SUPPORT
#  define RFM12_SHARE_UIP_BUF
#  undef RFM12_DataLength
#  define RFM12_DataLength (uint8_t)(UIP_CONF_BUFFER_SIZE - 10)
#  define RFM12_Data uip_buf
#endif /* no ENC28J60_SUPPORT */

#ifndef RFM12_SHARE_UIP_BUF
volatile uint8_t RFM12_Data[RFM12_DataLength + 10];
#endif


SIGNAL(RFM12_INT_SIGNAL)
{
  if(RFM12_status.Rx)
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
	  RFM12_status.Rx = 0;	/* FIXME this kills ourself */
	}

      if(RFM12_Index >= RFM12_Data[0] + 1)
	{
	  rfm12_trans(0x8208);
	  RFM12_status.Rx = 0;
	  RFM12_status.New = 1;
	}
    }

  else if(RFM12_status.Tx)
    {
      rfm12_trans(0xB800 | RFM12_Data[RFM12_Index]);

      if(RFM12_Index > RFM12_Data[5] + 8)
	{
	  RFM12_status.Tx = 0;

#ifdef RFM12_BLINK_PORT
	  RFM12_BLINK_PORT &= ~RFM12_TX_PIN;
#endif

	  rfm12_trans(0x8208);	/* TX off */
	  rfm12_rxstart();
	}
      else
        RFM12_Index++;
    }
  else
    {
      rfm12_trans(0x0000);	/* dummy read */
      /* FIXME what happend */
    }
}


unsigned short
rfm12_trans(unsigned short wert)
{	
  unsigned short werti = 0;
	
  SPI_CS_RFM12_PORT &= ~_BV(SPI_CS_RFM12);
	
  /* spi clock down */
  _SPCR0 |= _BV(SPR0);

  werti = (spi_send ((0xFF00 & wert) >> 8) << 8);
  werti += spi_send (0x00ff & wert);

  /* spi clock high */
  _SPCR0 &= ~_BV(SPR0);

  SPI_CS_RFM12_PORT |= _BV(SPI_CS_RFM12);
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
  
  RFM12_status.Rx = 0;
  RFM12_status.Tx = 0;
  RFM12_status.New = 0;

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
  if(RFM12_status.New)
    return(1);			/* buffer not yet empty */

  if(RFM12_status.Tx)
    return(2);			/* tx in action */

  if(RFM12_status.Rx)
    return(3);			/* rx already in action */
  
  rfm12_prologue ();

  rfm12_trans(0x82C8);		/* RX on */
  rfm12_trans(0xCA81);		/* set FIFO mode */
  rfm12_trans(0xCA83);		/* enable FIFO */

  rfm12_epilogue ();

  RFM12_Index = 0;
  RFM12_status.Rx = 1;
	
  return(0);
}


uint8_t 
rfm12_rxfinish(uint8_t *data)
{
  if(RFM12_status.Rx)
    return(255);		/* not finished yet */
  if(!RFM12_status.New)
    return(254);		/* old buffer */

  RFM12_status.New = 0;

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

  return(len);                 /* receive size */
}


uint8_t 
rfm12_txstart(uint8_t *data, uint8_t size)
{
  uint8_t i, l;

  if(RFM12_status.Tx)
    return(2);			/* tx in action */

  if(RFM12_status.Rx && RFM12_Index > 0)
    return(3);                  /* rx already in action */

  if(size > RFM12_DataLength)
    return(4);			/* str to big to transmit */

  RFM12_status.Rx = 0;
  RFM12_status.Tx = 1;

#ifdef RFM12_BLINK_PORT
  RFM12_BLINK_PORT |= RFM12_TX_PIN;
#endif

  i = size; while (i --)
              RFM12_Data[i + 6] = data[i];

  i = RFM12_Index = 0;

  RFM12_Data[i++] = 0xAA;
  RFM12_Data[i++] = 0xAA;
  RFM12_Data[i++] = 0xAA;
  RFM12_Data[i++] = 0x2D;
  RFM12_Data[i++] = 0xD4;
  RFM12_Data[i++] = size;
  i += size;
  RFM12_Data[i++] = 0xAA;
  RFM12_Data[i++] = 0xAA;

  rfm12_prologue ();
  rfm12_trans(0x8238);		/* TX on */
  rfm12_epilogue ();

  return(0);
}

