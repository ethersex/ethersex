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

#include "rfm12.h"


#ifdef RFM12_INTERRUPT
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

#ifdef RFADDR
volatile uint8_t RFM12_Ackdata = 0;
volatile uint16_t RFM12_delaycount = 0;
volatile uint8_t RFM12_maxcount = 0;
uint8_t RFM12_Data[RFM12_DataLength+12];  // +12 == paket overhead
#else
uint8_t RFM12_Data[RFM12_DataLength+10];	// +10 == paket overhead
#endif
#endif

//##############################################################################
//
#ifdef RFM12_INTERRUPT
SIGNAL(SIG_INTERRUPT0)
//##############################################################################
{
  if(RFM12_status.Rx)
    {
      if(RFM12_Index < RFM12_DataLength){
	RFM12_Data[RFM12_Index++] = rfm12_trans(0xB000) & 0x00FF;
	PORTB |= _BV(PB7);
      }
      else
	{
	  rfm12_trans(0x8208);
	  RFM12_status.Rx = 0;
	}
#ifdef RFADDR
      if(RFM12_Index >= RFM12_Data[0] + 3 +2)		//EOT
#else
	if(RFM12_Index >= RFM12_Data[0] + 3)    //EOT
#endif
	  {
	    rfm12_trans(0x8208);
	    RFM12_status.Rx = 0;
	    RFM12_status.New = 1;
	    //GICR &= ~(1<<INT0);		//disable int0
	  }
    }
  else if(RFM12_status.Tx)
    {
      rfm12_trans(0xB800 | RFM12_Data[RFM12_Index]);
      if(!RFM12_Index)
	{
	  RFM12_status.Tx = 0;
	  PORTB &= ~_BV(PB6);    // tx led aus
	  rfm12_trans(0x8208);		// TX off
	  rfm12_rxstart();
	}
      else
	{
	  RFM12_Index--;
	}
    }
  else
    {
      rfm12_trans(0x0000);			//dummy read
      //TODO: what happend
    }
}
#endif

//##############################################################################
//
unsigned int crcUpdate(unsigned int crc, uint8_t serialData)
//##############################################################################
{
  unsigned int tmp;
  uint8_t j;

  tmp = serialData << 8;
  for (j=0; j<8; j++)
    {
      if((crc^tmp) & 0x8000)
	crc = (crc<<1) ^ 0x1021;
      else
	crc = crc << 1;
      tmp = tmp << 1;
    }
  return crc;
}

//##############################################################################
//
unsigned short rfm12_trans(unsigned short wert)
//##############################################################################
{	
  unsigned short werti = 0;
	
  RF_PORT &=~(1<<CS);
	
#ifdef SPI_MODE	//Routine f�r Hardware SPI
  SPDR = (0xFF00 & wert)>>8;
  while(!(SPSR & (1<<SPIF))){};
  werti = (SPDR<<8);
	
  SPDR = (0x00ff & wert);
  while(!(SPSR & (1<<SPIF))){};
  werti = werti + SPDR;

#else			//Routine f�r Software SPI
  for (uint8_t i=0; i<16; i++)
    {	
      if (wert&32768)
	RF_PORT |=(1<<SDI);
      else
	RF_PORT &=~(1<<SDI);
      werti<<=1;
      if (RF_PIN&(1<<SDO))
	werti|=1;
      RF_PORT |=(1<<SCK);
      wert<<=1;
      asm("nop");
      RF_PORT &=~(1<<SCK);
    }
#endif
  RF_PORT |=(1<<CS);
  return werti;
}

//##############################################################################
//
void rfm12_init(void)
//##############################################################################
{
  uint8_t i;

  RF_DDR |= (1<<SDI)|(1<<SCK)|(1<<CS);
  RF_DDR &= ~(1<<SDO);
  RF_PORT |= (1<<CS);
#ifdef SPI_MODE
  //Aktiviren des SPI - Bus, Clock = Idel LOW
  //SPI Clock teilen durch 128, Enable SPI, SPI in Master Mode
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
  SPSR &= ~(0<<SPI2X);
#endif
  
  for (i=0; i<10; i++)
    _delay_ms(10);			// wait until POR done
  rfm12_trans(0xC0E0);			// AVR CLK: 10MHz
  rfm12_trans(0x80D7);			// Enable FIFO
  rfm12_trans(0xC2AB);			// Data Filter: internal
  rfm12_trans(0xCA81);			// Set FIFO mode
  rfm12_trans(0xE000);			// disable wakeuptimer
  rfm12_trans(0xC800);			// disable low duty cycle
  rfm12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz
  rfm12_trans(0x0000);
  
#ifdef RFM12_INTERRUPT
  RFM12_status.Rx = 0;
  RFM12_status.Tx = 0;
  RFM12_status.New = 0;

  RF_IRQDDR &= ~(1<<IRQ);
  GICR |= (1<<INT0);
#endif
}

//##############################################################################
//
void rfm12_setbandwidth(uint8_t bandwidth, uint8_t gain, 
			uint8_t drssi)
//##############################################################################
{
  rfm12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

//##############################################################################
//
void rfm12_setfreq(unsigned short freq)
//##############################################################################
{	
  if (freq<96)					// 430,2400MHz
    freq=96;
  else if (freq>3903)			// 439,7575MHz
    freq=3903;
  rfm12_trans(0xA000|freq);
}

//##############################################################################
//
void rfm12_setbaud(unsigned short baud)
//##############################################################################
{
  if (baud<663)
    return;
  if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
    rfm12_trans(0xC680|((43104/baud)-1));
  else
    rfm12_trans(0xC600|((344828UL/baud)-1));
}

//##############################################################################
//
void rfm12_setpower(uint8_t power, uint8_t mod)
//##############################################################################
{	
  rfm12_trans(0x9800|(power&7)|((mod&15)<<4));
}

//##############################################################################
//
#ifdef RFM12_INTERRUPT
uint8_t rfm12_rxstart(void)
//##############################################################################
{
  if(RFM12_status.New)
    return(1);			//buffer not yet empty
  if(RFM12_status.Tx)
    return(2);			//tx in action
  if(RFM12_status.Rx)
    return(3);			//rx already in action
  
  rfm12_trans(0x82C8);			// RX on
  rfm12_trans(0xCA81);			// set FIFO mode
  rfm12_trans(0xCA83);			// enable FIFO

  RFM12_Index = 0;
  RFM12_status.Rx = 1;
	
  return(0);				//all went fine
}

//##############################################################################
//
#ifdef RFADDR
uint8_t rfm12_rxfinish(uint8_t *txaddr, uint8_t *data)
#else
  uint8_t rfm12_rxfinish(uint8_t *data)
#endif
//##############################################################################
{
  unsigned int crc, crc_chk = 0;
  uint8_t i;
  if(RFM12_status.Rx)
    return(255);				//not finished yet
  if(!RFM12_status.New)
    return(254);				//old buffer
#ifdef RFADDR
  for(i=0; i<RFM12_Data[0] +1 +2 ; i++)
#else
  for(i=0; i<RFM12_Data[0] +1; i++)
#endif
    crc_chk = crcUpdate(crc_chk, RFM12_Data[i]);

  crc = RFM12_Data[i++];
  crc |= RFM12_Data[i] << 8;
  RFM12_status.New = 0;
  PORTB &= ~_BV(PB7);
#ifdef RFADDR
  if(crc != crc_chk || RFM12_Data[2] != RFADDR) {
#else
  if(crc != crc_chk) {
#endif
    rfm12_rxstart();
    return(0);				//crc err -or- strsize
  }
  else {
    uint8_t i;
#ifdef RFADDR
    if(RFM12_status.Ack == 1 && RFM12_Data[0] == 1) {
      RFM12_status.Ack = 0;
      rfm12_rxstart();
      return(0);
    }
    *txaddr = RFM12_Data[1];
    for(i=0; i<RFM12_Data[0]; i++)
      data[i] = RFM12_Data[i+1+2];
#else
    for(i=0; i<RFM12_Data[0]; i++)
      data[i] = RFM12_Data[i+1];
#endif
    i =  RFM12_Data[0];
#ifdef RFADDR
    if(i > 1)
      rfm12_txstart(*txaddr, data,1);
    else
#endif
      rfm12_rxstart();
    return(i);			//strsize
  }
}

//##############################################################################
//
#ifdef RFADDR
uint8_t rfm12_txstart(uint8_t txaddr, uint8_t *data, uint8_t size)
#else
uint8_t rfm12_txstart(uint8_t *data, uint8_t size)
#endif
//##############################################################################
{
  uint8_t i, l;
  unsigned int crc;
  if(RFM12_status.Tx)
    return(2);			//tx in action
  if(RFM12_status.Rx && RFM12_Index > 0)//if(RFM12_status.Rx)
    return(3);   //rx already in action
  if(size > RFM12_DataLength)
    return(4);			//str to big to transmit
  RFM12_status.Rx = 0;
  //PORTD &= ~_BV(PD7); //rx led off
  PORTB |= _BV(PB6); //tx led on
  
  RFM12_status.Tx = 1;
#ifdef RFADDR
  if(size > 1) {
    RFM12_status.Ack = 1;
    RFM12_Ackdata = data[0];
  }
  RFM12_Index = size + 9 + 2;     //act -12
#else
  RFM12_Index = size + 9;			//act -10 
#endif
  
  i = RFM12_Index;				
  RFM12_Data[i--] = 0xAA;
  RFM12_Data[i--] = 0xAA;
  RFM12_Data[i--] = 0xAA;
  RFM12_Data[i--] = 0x2D;
  RFM12_Data[i--] = 0xD4;
  RFM12_Data[i--] = size;
  crc = crcUpdate(0, size);
#ifdef RFADDR
  RFM12_Data[i--] = RFADDR;
  crc = crcUpdate(crc, RFADDR);
  RFM12_Data[i--] = txaddr;
  crc = crcUpdate(crc, txaddr);
#endif
  for(l=0; l<size; l++) {
    RFM12_Data[i--] = data[l];
    crc = crcUpdate(crc, data[l]);
  }	
  RFM12_Data[i--] = (crc & 0x00FF);
  RFM12_Data[i--] = (crc >> 8);
  RFM12_Data[i--] = 0xAA;
  RFM12_Data[i--] = 0xAA;

  rfm12_trans(0x8238);			// TX on
  
  return(0);				//all went fine
}

//##############################################################################
//
uint8_t rfm12_txfinished(void)
//##############################################################################
{
  if(RFM12_status.Tx)
    return(255);			//not yet finished
  return(0);
}

uint8_t rfm12_Index(void)
{
  return (RFM12_Index);
}

//##############################################################################
//
void rfm12_allstop(void)
//##############################################################################
{
  //GICR &= ~(1<<INT0);		//disable int0	
  RFM12_status.Rx = 0;
  RFM12_status.Ack = 0;
  RFM12_status.Tx = 0;
  RFM12_status.Txok = 0;
  RFM12_status.New = 0;
  rfm12_trans(0x8208);		//shutdown all
  rfm12_trans(0x0000);		//dummy read
}

#ifdef RFADDR
uint8_t rfm12_txto(uint8_t txaddr, uint8_t *txdata, uint8_t len)
{
  if(len == 1)
    return rfm12_txstart(txaddr, txdata, len);
  if( RFM12_status.Txok == 0){
    rfm12_txstart(txaddr, txdata, len);
    RFM12_status.Txok = 1;
    RFM12_delaycount = 0;
    RFM12_maxcount = 0;
    return (12); //tx gestartet warte auf ack
  }
  else{
    if(RFM12_status.Ack == 1){
      //_delay_ms(25);
      if(RFM12_delaycount++ == 0x8fff){
	RFM12_delaycount = 0;
	if(RFM12_maxcount++ == 10){
	  RFM12_status.Txok = 0;
	  return(1); //zu oft probiert, breche ab
	}
	rfm12_txstart(txaddr, txdata, len);
      }
      return (11); //nochmal versuchen
    }
    else{
      RFM12_status.Txok = 0;
      return (0); //ack empfangen
    }
  }

}
#endif

//##############################################################################
//
#else
void rfm12_ready(void)
//##############################################################################
{
  unsigned long timeout = 0;
  RF_PORT &=~(1<<CS);
  asm("nop");
  asm("nop");
  asm("nop");
  //wait until FIFO ready or timeout
  while (!(RF_PIN & (1<<SDO)))
    {
      if ((timeout++)>100000){break;};
    };
  RF_PORT |=(1<<CS);
}

//##############################################################################
//
#ifdef RFADDR
void rfm12_txdata(uint8_t txaddr, uint8_t *data, uint8_t number)
#else
void rfm12_txdata(uint8_t *data, uint8_t number)
#endif
//##############################################################################
{	
  PORTB |= _BV(PB6);
  uint8_t i;
  unsigned int crc;
  rfm12_trans(0x8238);			// TX on

  rfm12_ready();
  rfm12_trans(0xB8AA);
  rfm12_ready();
  rfm12_trans(0xB8AA);
  rfm12_ready();
  rfm12_trans(0xB8AA);
  rfm12_ready();
  rfm12_trans(0xB82D);
  rfm12_ready();
  rfm12_trans(0xB8D4);
  /*#ifdef RFADDR
    rfm12_ready();
    rfm12_trans(0xB800 | (number +2)); 
    crc = crcUpdate(0, number +2);
    #else*/
  rfm12_ready();
  rfm12_trans(0xB800 | number); 
  crc = crcUpdate(0, number);
  // #endif
#ifdef RFADDR
  rfm12_ready();
  rfm12_trans(0xB800 | RFADDR);
  crc = crcUpdate(crc, RFADDR);
  rfm12_ready();
  rfm12_trans(0xB800 | txaddr);
  crc = crcUpdate(crc, txaddr);
#endif
  for (i=0; i<number; i++)
    {	
      rfm12_ready();
      rfm12_trans(0xB800 | data[i]);
      crc = crcUpdate(crc, data[i]);
    }
  rfm12_ready();
  rfm12_trans(0xB800 | (crc & 0x00FF));
  rfm12_ready();
  rfm12_trans(0xB800 | (crc >> 8));
  rfm12_ready();
  rfm12_trans(0xB8AA);
  rfm12_ready();
  rfm12_trans(0xB8AA);
  rfm12_ready();

  rfm12_trans(0x8208);			// TX off
  PORTB &= ~_BV(PB6);
}

//##############################################################################
//
#ifdef RFADDR
uint8_t rfm12_rxdata(uint8_t *txaddr, uint8_t *data)
#else
uint8_t rfm12_rxdata(uint8_t *data)
#endif
//##############################################################################
{	
  uint8_t i, number, rxaddr = 0, txaddrget = 0;
  unsigned int crc, crc_chk;
  rfm12_trans(0x82C8);			// RX on
  rfm12_trans(0xCA81);			// set FIFO mode
  rfm12_trans(0xCA83);			// enable FIFO
	
  rfm12_ready();
  number = rfm12_trans(0xB000) & 0x00FF;
  if (number > 200) number = 200;
  crc_chk = crcUpdate(0, number);

#ifdef RFADDR
  if (number > 0){
    PORTB |= _BV(PB7);
    rfm12_ready();
    txaddrget = rfm12_trans(0xB000) & 0x00FF;
    crc_chk = crcUpdate(crc_chk, txaddrget);
    rfm12_ready();
    rxaddr = rfm12_trans(0xB000) & 0x00FF;
    crc_chk = crcUpdate(crc_chk, rxaddr);
  }
#endif

  for (i=0; i<number; i++)
    {
      rfm12_ready();
      data[i] = (uint8_t) (rfm12_trans(0xB000) & 0x00FF);
      crc_chk = crcUpdate(crc_chk, data[i]);
    }
  data[number+1] = 0;
  rfm12_ready();
  crc = rfm12_trans(0xB000) & 0x00FF;
  rfm12_ready();
  crc |=  rfm12_trans(0xB000) << 8;

  rfm12_trans(0x8208);			// RX off
  PORTB &= ~_BV(PB7);
  if (crc != crc_chk)
    number = 0;
#ifdef RFADDR
  else if (rxaddr != RFADDR)
    number = 0;
  else if(number > 1){
    //uint8_t ackdata[2];
    //ackdata[0] = data[0];
    //ackdata[1] = data[2];
    _delay_ms(25);
    _delay_ms(25);
    rfm12_txdata(txaddrget, &data[0],1);
  }
  if(number > 0)
    *txaddr = txaddrget;
#endif
  return number;
}

#ifdef RFADDR
/* paket an eine bestimmte adresse schicken und auf bestaetigung warten */
uint8_t rfm12_sendto(uint8_t txaddr, uint8_t *rxbuf, uint8_t *txdata,
		     uint8_t len)
{
  memset(rxbuf, 0, 3);
  uint8_t maxtrytx = 15;
  uint8_t maxtryrx;
  uint8_t rxlen = 0;
  uint8_t rxaddr;
  do {
    maxtryrx = 2;
    rfm12_txdata(txaddr, txdata, len);
    do {
      rxlen = rfm12_rxdata(&rxaddr, rxbuf);
    }
    while(rxlen != 1 && txaddr != rxaddr && maxtryrx-- > 0);
  }
  while(rxaddr != RFADDR && rxbuf[0] != txdata[0] && maxtrytx-- > 0);
  
  return maxtrytx;
}

#endif

#endif
