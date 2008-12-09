dnl
dnl atmega32.m4
dnl
dnl   Copyright (c) 2008 by Georg von Zengen <oni@chaosplatz.com>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by 
dnl   the Free Software Foundation; either version 2 of the License, or
dnl   (at your option) any later version.
dnl  
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl  
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl 

/* spi defines */
pin(SPI_MOSI, PB5)
pin(SPI_MISO, PB6)
pin(SPI_SCK, PB7)
pin(SPI_CS_HARDWARE, PB4)

ifdef(`conf_RFM12', `dnl
dnl
dnl Ancient zerties.org RFM12 pinout:
dnl   chip select: PC3
dnl   tx-led: 	   PD4
dnl   rx-led:	   PD5
dnl   interrupt:   INT0
dnl
dnl Configuration suggest for Etherrape hardware
dnl (all pins available at SPI connector)
dnl 
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB0)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, PB3)
  pin(RFM12_RX_PIN, PB1)

  RFM12_USE_INT(2)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PC2)
  pin(ZBUS_TX_PIN, PD4)
  pin(ZBUS_RX_PIN, PD5)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
  pin(ECMD_SERIAL_USART_TX, PC2)
')

ifdef(`conf_STELLA', `dnl
  STELLA_PORT_RANGE(PD5,PD7)
')

#define NET_MAX_FRAME_LENGTH 500
#define ADC_CHANNELS 8
