dnl
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
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl

dnl  Some macros that you might find useful if you're rewriting packets.

define(`REWRITE_SRCADDR', `uip_ipaddr_copy(BUF->srcipaddr, $1);');
define(`REWRITE_DESTADDR', `uip_ipaddr_copy(BUF->destipaddr, $1);');

define(`REWRITE_CHKSUM_IP', `dnl
#ifndef IPV6_SUPPORT
  BUF->ipchksum = 0;
  BUF->ipchksum = ~(uip_ipchksum());
#endif
')
