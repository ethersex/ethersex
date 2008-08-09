dnl
dnl atmega8.m4
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
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB2)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, PD6)
  pin(RFM12_RX_PIN, PD7)

  RFM12_USE_INT(0)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  pin(ZBUS_TX_PIN, PD6)
  pin(ZBUS_RX_PIN, PD7)
')

ifdef(`conf_STELLA', `dnl
  STELLA_PORT_RANGE(PB0,PB3)
')

#define _ATMEGA8

/* ATmega8 specific adjustments */
#define _SPCR0 SPCR
#define _SPE0 SPE
#define _MSTR0 MSTR
#define _SPSR0 SPSR
#define _SPIF0 SPIF
#define _SPDR0 SPDR
#define _SPI2X0 SPI2X
#define _TIFR_TIMER1 TIFR
#define _EIMSK GICR
#define _UDR_UART0 UDR
#define _UCSRA_UART0 UCSRA
#define _UCSRB_UART0 UCSRB
#define _UCSRC_UART0 UCSRC
#define _UBRRL_UART0 UBRRL
#define _UBRRH_UART0 UBRRH
#define _TXEN_UART0  TXEN
#define _TXCIE_UART0 TXCIE
#define _RXEN_UART0  RXEN
#define _RXCIE_UART0 RXCIE
#define _UDRE_UART0  UDRE
#define _RXC_UART0   RXC
#define _TXC_UART0   TXC
#define UDRIE0       UDRIE
#define DOR0         DOR
#define FE0          FE
#define UCSZ00       UCSZ0
#define UCSZ01       UCSZ1
#define USART0_UDRE_vect USART_UDRE_vect
#define USART0_RX_vect USART_RXC_vect
#define USART0_TX_vect USART_TXC_vect

#define _TCCR2_PRESCALE TCCR2
#define _OUTPUT_COMPARE_IE2 OCIE2
#define _OUTPUT_COMPARE_REG2 OCR2
#define _SIG_OUTPUT_COMPARE2 SIG_OUTPUT_COMPARE2
#define _SIG_OVERFLOW2 SIG_OVERFLOW2
#define _TIMSK_TIMER2 TIMSK

#define BOOTLOADER_SECTION 0x0E00 /* atmega8 with 256 words bootloader */


#define HD44780_CTRL_PORT D
#define HD44780_DATA_PORT D
#define HD44780_RS PD0
#define HD44780_EN PD1
#define HD44780_D4 PD4
#define HD44780_D5 PD5
#define HD44780_D6 PD6
#define HD44780_D7 PD7
#define HD44780_DATA_SHIFT 4


/* there isn't that much RAM on ATmega8, reduce uip_buf size. */
#define NET_MAX_FRAME_LENGTH 192

/* on the ATmega8 we only have 6 adc channels in the pdip version */
#define ADC_CHANNELS 6
