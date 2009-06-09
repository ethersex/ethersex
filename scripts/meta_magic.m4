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
define(`timer_divert',10)dnl after timer divert there musn't be any other divert level
divert(0)dnl
/* This file has been generated automatically.
   Please do not modify it, edit the m4 scripts instead. */

#include <avr/wdt.h>
#include "config.h"
void dyndns_update();
void periodic_process();
extern uint8_t bootload_delay;

divert(initearly_divert)dnl
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
    periodic_process(); wdt_kick();
}

divert(-1)dnl

define(`header', `divert(prototypes)#include "$1"')
define(`prototype', `divert(prototypes)$1;')

define(`init',`dnl
dnl divert(prototypes)void $1 (void);
divert(init_divert)    $1 ();
divert(-1)');

define(`initearly',`dnl
dnl divert(prototypes)void $1 (void);
divert(initearly_divert)    $1 ();
divert(-1)');

define(`net_init',`dnl
dnl divert(prototypes)void $1 (void);
divert(net_init_divert)    $1 ();
divert(-1)');

define(`startup',`dnl
dnl divert(prototypes)void $1 (void);
divert(startup_divert)    $1 (); wdt_kick ();
divert(-1)');

define(`mainloop',`dnl
dnl divert(prototypes)void $1 (void);
divert(mainloop_divert)    $1 (); wdt_kick ();
divert(-1)');

dnl 
dnl Timer foo
dnl

define(`pushdivert', `define(`_old_divert', divnum)')
define(`popdivert', `divert(_old_divert)')
define(`timer_divert_base', timer_divert)
define(`timer_divert_last', 500)
define(`timer_divert_start', `divert(eval(timer_divert_base` + $1 * 2'))$2')
define(`timer_divert_end', `divert(eval(timer_divert_base` + $1 * 2 + 1'))$2')
define(`_divert_used', `ifelse(eval(`$1 > 'timer_divert_last), `1', `errprint(`timer_meta: Too big timer $1
')m4exit(1)')ifdef(`_divert_used_$1', `', `define(`_divert_used_$1', `1')
timer_divert_start($1, `
if (counter % $1 == 0) {
')dnl
timer_divert_end($1, `}
')dnl
')')
define(`timer', `pushdivert()_divert_used($1)timer_divert_start($1, `$2;
')popdivert()')
divert(timer_divert_base)
void periodic_process(void)
{
    static uint16_t counter = 0;
    if (_TIFR_TIMER1 & _BV(OCF1A)) {
        /* clear flag */
        _TIFR_TIMER1 = _BV(OCF1A);
        counter++;
#ifdef UIP_SUPPORT
        if (uip_buf_lock ()) {
#ifdef RFM12_IP_SUPPORT
           _uip_buf_lock --;
           if (uip_buf_lock ()) {
             return;           /* hmpf, try again shortly
                                   (let's hope we don't miss too many ticks */
           }
           else {
               rfm12_status = RFM12_OFF;
               rfm12_rxstart();
           }
#else
           return;
#endif
        }
#endif

divert(eval(timer_divert_base`+'timer_divert_last` * 2 + 2'))

#ifdef  UIP_SUPPORT
   uip_buf_unlock ();
#endif

#       ifdef BOOTLOADER_SUPPORT
        if(bootload_delay)
            if(-- bootload_delay == 0) {
		cli();
		_IVREG = _BV(IVCE);	        /* prepare ivec change */
		_IVREG = 0x00;                  /* change ivec */

                void (*jump_to_application)(void) = NULL;
                jump_to_application();
            }
#       endif
    }
}
divert(-1)
timer(timer_divert_last, `counter = 0')
