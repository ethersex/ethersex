dnl
dnl footer.m4
dnl
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

#define PORTIO_MASK_A eval(255 - port_mask_A)
#define PORTIO_MASK_B eval(255 - port_mask_B)
#define PORTIO_MASK_C eval(255 - port_mask_C)
#define PORTIO_MASK_D eval(255 - port_mask_D)

