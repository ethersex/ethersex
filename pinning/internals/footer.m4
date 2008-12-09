dnl
dnl footer.m4
dnl
dnl   Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
dnl   Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#define PORTIO_MASK_A eval(255 - port_mask_A)
#define PORTIO_MASK_B eval(255 - port_mask_B)
#define PORTIO_MASK_C eval(255 - port_mask_C)
#define PORTIO_MASK_D eval(255 - port_mask_D)

dnl
dnl  We have to make sure, that we configure SPI_CS_HARDWARE as output
dnl  if none of the other SPI_CS-lines uses it...
dnl

/* chip-select-voodoo:    SPI_CS_HARDWARE
   SPI_CS_DF SPI_CS_NET SPI_CS_RFM12 SPI_CS_SD_READER */

ifelse(SPI_CS_HARDWARE,SPI_CS_DF,`define(SPI_CS_HARDWARE_USED,1)',`/* DF differs */')
ifelse(SPI_CS_HARDWARE,SPI_CS_NET,`define(SPI_CS_HARDWARE_USED,1)',`/* NET differs */')
ifelse(SPI_CS_HARDWARE,SPI_CS_RFM12,`define(SPI_CS_HARDWARE_USED,1)',`/* RFM12 differs */')
ifelse(SPI_CS_HARDWARE,SPI_CS_SD_READER,`define(SPI_CS_HARDWARE_USED,1)',`/* SD-Reader differs */')

ifelse(SPI_CS_HARDWARE_USED,1, ` 
  /* hardware chip select is used by some configured CS, nothing to do ... */
  #define SPI_NO_EXTRA_HARDWARE_CS_CONFIG 1
')
