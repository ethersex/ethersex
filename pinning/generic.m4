dnl
dnl generic.m4
dnl
dnl   Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
dnl   Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
dnl   Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 2 as
dnl   published by the Free Software Foundation.
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
divert(0)dnl
dnl

ifdef(`conf_ENC28J60', `dnl
  /* port the enc28j60 is attached to
   * ATTENTION: EITHER USE SS OR MAKE SURE, SS IS PULLED HIGH OR AN OUTPUT! */

  pin(SPI_CS_NET, PB4)
')dnl

ifdef(`conf_DATAFLASH', `dnl
  /* port the dataflash CS is attached to */
  pin(SPI_CS_DF, PB1)
')dnl

ifdef(`conf_MODBUS', `dnl
  /* modbus tx  */
  pin(MODBUS_TX, PC2)
')dnl

ifdef(`conf_PS2', `dnl
  /* ps/2 pins */
  pin(PS2_DATA, PA7)
  pin(PS2_CLOCK, PA6)
')dnl

ifdef(`conf_HC595', `dnl
  /* pins for the hc595 shift register */
  pin(HC595_DATA, PB6)
  pin(HC595_CLOCK, PB2)
  pin(HC595_STORE, PB7)
')dnl

ifdef(`conf_HC165', `dnl
  /* pins for the hc165 shift register */
  pin(HC165_DATA, PB0)
  pin(HC165_CLOCK, PB2)
  pin(HC165_LOAD, PB1)
')dnl

dnl /* enc28j60 int line */
dnl pin(INT_PIN, PB3)
dnl /* enc28j60 wol line */
dnl pin(WOL_PIN, PB2)

ifdef(`conf_ONEWIRE', `dnl
  /* onewire support */
  pin(ONEWIRE, PD6)
')dnl

ifdef(`conf_RC5', `dnl
  /* rc5 support */
  pin(RC5_SEND, PD4)
')dnl

ifdef(`conf_FS20',  `dnl
  /* fs20 support */
  pin(FS20_SEND, PB2)
  /* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
  pin(FS20_RECV, PB3)
')dnl
