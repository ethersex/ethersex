dnl
dnl atmega88.m4
dnl
dnl   Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
dnl   Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
dnl   Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by 
dnl   the Free Software Foundation; either version 2 of the License, or
dnl   (at your option) any later version.
dnl  
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

ifdef(`conf_RFM12', `define(need_spi, 1)')dnl
ifdef(`conf_ENC28J60', `define(need_spi, 1)')dnl

ifdef(`need_spi', `dnl
  /* spi defines */
  pin(SPI_MOSI, PB3)
  pin(SPI_MISO, PB4)
  pin(SPI_SCK, PB5)
  pin(SPI_CS_HARDWARE, PB2)
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB2)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, PD6)
  pin(RFM12_RX_PIN, PD7)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  pin(ZBUS_TX_PIN, PD6)
  pin(ZBUS_RX_PIN, PD7)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
  pin(ECMD_SERIAL_USART_TX, PD2)
')

/* there isn't that much RAM on ATmega88, reduce uip_buf size. */
#define NET_MAX_FRAME_LENGTH 192
