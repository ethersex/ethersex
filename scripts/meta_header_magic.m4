dnl
dnl   Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl   Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 3 as
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
dnl   For more information on the GPL, please go to:
dnl   http://www.gnu.org/copyleft/gpl.html
dnl
define(`prototypes', 0)dnl
define(`udp_state_divert', 10)dnl
define(`tcp_state_divert', 11)dnl
define(`tcp_state_end_divert', 12)dnl
divert(0)dnl
/* This file has been generated automatically.
   Please do not modify it, edit the m4 scripts instead. */

#include "core/pt/pt.h"
#ifndef _META_H
#define _META_H

divert(udp_state_divert)dnl
/* uip appstate for udp */
typedef union uip_udp_connection_state {
#   ifdef CONTROL6_SUPPORT
    struct {
	struct pt pt;
    } control6_udp;
#   endif

divert(tcp_state_divert)dnl
} uip_udp_appstate_t; /* end of appstate for udp */

/* uip appstate for tcp */
typedef union uip_tcp_connection_state {
#   ifdef CONTROL6_SUPPORT
    struct {
	struct pt pt;
	lc_t rexmit_lc;
    } control6_tcp;
#   endif

divert(tcp_state_end_divert)dnl
} uip_tcp_appstate_t; /* end of tcp appstate */

#endif /* _META_H */

divert(-1)dnl
define(`state_header', `divert(prototypes)#include "$1"
divert(-1)')

define(`state_udp',`dnl
divert(udp_state_divert)    $1;
divert(-1)');

define(`state_tcp',`dnl
divert(tcp_state_divert)    $1;
divert(-1)');

