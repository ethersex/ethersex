dnl
dnl   Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
define(`prototypes',0)dnl
define(`initearly_divert',1)dnl
define(`init_divert',2)dnl
define(`net_init_divert',3)dnl
define(`startup_divert',4)dnl
define(`mainloop_divert',5)dnl
divert(0)dnl
/* This file has been generated automatically.
   Please do not modify it, edit the m4 scripts instead. */

#include <avr/wdt.h>
#include "config.h"
void dyndns_update();

divert(init_divert)dnl
void
ethersex_meta_init (void)
{
divert(net_init_divert)dnl
}  /* End of ethersex_meta_init. */

void
ethersex_meta_netinit (void)
{
#   if defined(DYNDNS_SUPPORT) && !defined(BOOTP_SUPPORT) \
      && ((!defined(IPV6_SUPPORT)) || defined(IPV6_STATIC_SUPPORT))
    dyndns_update();
#   endif

divert(startup_divert)dnl
}  /* End of ethersex_meta_netinit. */

void
ethersex_meta_startup (void)
{

divert(mainloop_divert)dnl
}  /* End of ethersex_meta_startup. */

void
ethersex_meta_mainloop (void)
{

divert(9)dnl
}

divert(-1)dnl

define(`init',`dnl
divert(prototypes)void $1 (void);
divert(init_divert)    $1 ();
divert(-1)');

define(`initearly',`dnl
divert(prototypes)void $1 (void);
divert(initearly_divert)    $1 ();
divert(-1)');

define(`net_init',`dnl
divert(prototypes)void $1 (void);
divert(net_init_divert)    $1 ();
divert(-1)');

define(`startup',`dnl
divert(prototypes)void $1 (void);
divert(startup_divert)    $1 (); wdt_kick ();
divert(-1)');

define(`mainloop',`dnl
divert(prototypes)void $1 (void);
divert(mainloop_divert)    $1 (); wdt_kick ();
divert(-1)');
