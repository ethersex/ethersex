dnl
dnl meduino.m4
dnl
dnl   Copyright (c) 2016 by Erik Kunze <ethersex@erik-kunze.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by 
dnl   the Free Software Foundation; either version 3 of the License, or
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

ifdef(`conf_STATUSLED_HB_ACT', `dnl
  pin(STATUSLED_HB_ACT, PB7, OUTPUT)
')dnl

