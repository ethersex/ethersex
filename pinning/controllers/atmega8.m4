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

ifdef(`need_spi', `dnl
  /* spi defines */
  pin(SPI_MOSI, PB3, OUTPUT)
  pin(SPI_MISO, PB4)
  pin(SPI_SCK, PB5, OUTPUT)
  pin(SPI_CS_HARDWARE, PB2, OUTPUT)
')dnl


ifdef(`conf_BLP', `dnl
/* Pinbelegung am Atmega8:
 * 
 * clock:  PB0
 * strobe: PD5
 * data 1/2: PB1
 * data 3/4: PB2
 * data 5/6: PD4
 * data 5/8: PD3
 * data 9/10: PB3
 * data 11/12: PB4
 * data 13/14: PB5
 * data 15/16: PC2
 * data 17/18: PC3 */
 pin(BLP_CLK, PB0, OUTPUT)
 pin(BLP_STR, PD5, OUTPUT)
 pin(BLP_DA_A, PB1, OUTPUT)
 pin(BLP_DA_B, PB2, OUTPUT)
 pin(BLP_DA_C, PD4, OUTPUT)
 pin(BLP_DA_D, PD3, OUTPUT)
 pin(BLP_DA_E, PB3, OUTPUT)
 pin(BLP_DA_F, PB4, OUTPUT)
 pin(BLP_DA_G, PB5, OUTPUT)
 pin(BLP_DA_H, PC2, OUTPUT)
 pin(BLP_DA_I, PC3, OUTPUT)
')

ifdef(`conf_I2C_MASTER', `
  /* I2C pins */
  pin(SDA, PC4)
  pin(SCL, PC5)
')dnl

/* on the ATmega8 we only have 6 adc channels in the pdip version */
#define ADC_CHANNELS 6

/* MCU doesn't support toggling pins by writing to PINx register */
#define PINx_TOGGLE_WORKAROUND

