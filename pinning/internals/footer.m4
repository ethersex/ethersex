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

divert(eof_divert)

#define PORTIO_MASK_A eval(255 - port_mask_A)
#define PORTIO_MASK_B eval(255 - port_mask_B)
#define PORTIO_MASK_C eval(255 - port_mask_C)
#define PORTIO_MASK_D eval(255 - port_mask_D)
#define PORTIO_MASK_E eval(255 - port_mask_E)
#define PORTIO_MASK_F eval(255 - port_mask_F)
#define PORTIO_MASK_G eval(255 - port_mask_G)
#define PORTIO_MASK_H eval(255 - port_mask_H)
#define PORTIO_MASK_J eval(255 - port_mask_J)
#define PORTIO_MASK_K eval(255 - port_mask_K)
#define PORTIO_MASK_L eval(255 - port_mask_L)

#define DDR_MASK_A ddr_mask_A
#define DDR_MASK_B ddr_mask_B
#define DDR_MASK_C ddr_mask_C
#define DDR_MASK_D ddr_mask_D
#define DDR_MASK_E ddr_mask_E
#define DDR_MASK_F ddr_mask_F
#define DDR_MASK_G ddr_mask_G
#define DDR_MASK_H ddr_mask_H
#define DDR_MASK_J ddr_mask_J
#define DDR_MASK_K ddr_mask_K
#define DDR_MASK_L ddr_mask_L

#endif /* _PINNING_HEADER */
