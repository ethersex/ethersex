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
define(`prototypes',0)dnl
define(`initearly_divert',12)dnl
define(`init_divert',13)dnl
define(`net_init_divert',14)dnl
define(`exit_divert',15)dnl
define(`startup_divert',16)dnl
define(`mainloop_divert',17)dnl
define(`timer_divert',18)dnl after timer divert there musn't be any other divert level
divert(0)dnl
/* This file has been generated automatically.
   Please do not modify it, edit the m4 scripts instead. */

#include "config.h"

#if ARCH != ARCH_HOST
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#endif

#include <stdint.h>
#include "core/debug.h"
#include "services/freqcount/freqcount.h"

#if ARCH == ARCH_HOST
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "core/host/tap.h"
#include "core/host/stdin.h"

/* for C-c exit handler */
#include <signal.h>
#include <stdlib.h>
void ethersex_meta_exit (int signal);

#endif

void dyndns_update(void);
void periodic_process(void);
volatile uint8_t newtick;

divert(initearly_divert)dnl
void
ethersex_meta_init (void)
{
#if ARCH == ARCH_HOST
    signal(SIGINT, ethersex_meta_exit);
#endif

divert(net_init_divert)dnl
}  /* End of ethersex_meta_init. */

void
ethersex_meta_netinit (void)
{
#   if defined(DYNDNS_SUPPORT) && !defined(BOOTP_SUPPORT) \
      && ((!defined(IPV6_SUPPORT)) || defined(IPV6_STATIC_SUPPORT))
    dyndns_update();
#   endif

divert(exit_divert)dnl
} /* End of ethersex_meta_netinit. */

#if ARCH == ARCH_HOST
void
ethersex_meta_exit (int signal)
{
    if (signal != SIGINT) return;
    printf ("Shutting down Ethersex ...\n");
divert(startup_divert)dnl
    exit(0);
} /* End of ethersex_meta_exit. */
#endif  /* ARCH == ARCH_HOST */


void
ethersex_meta_startup (void)
{

divert(mainloop_divert)dnl
}  /* End of ethersex_meta_startup. */

void
ethersex_meta_mainloop (void)
{

divert(timer_divert)dnl
    periodic_process(); wdt_kick();

#ifdef FREQCOUNT_SUPPORT
    freqcount_mainloop();
#endif

#ifdef CPU_SLEEP
/* Works only if there are interrupts enabled, e.g. from periodic.c */
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
#endif
}

divert(-1)dnl

define(`header', `divert(prototypes)#include "$1"
divert(-1)')
define(`prototype', `divert(prototypes)$1;
divert(-1)')

define(`init',`dnl
dnl divert(prototypes)void $1 (void);
divert(init_divert)    $1 ();
divert(-1)');

define(`atexit', `dnl
divert(exit_divert)	$1 ();
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

define(`state_udp',`') dnl udp and tcp state is handled by meta_header_magic.m4
define(`state_tcp', `')

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
#if ARCH == ARCH_HOST
    {
	fd_set fds;
	struct timeval tv = { .tv_sec = 0, .tv_usec = 20000 };

	FD_ZERO (&fds);
#ifdef ECMD_PARSER_SUPPORT
	FD_SET (0, &fds);
#endif  /* ECMD_PARSER_SUPPORT */
	FD_SET (tap_fd, &fds);
	select (tap_fd + 1, &fds, NULL, NULL, &tv);

#ifdef ECMD_PARSER_SUPPORT
	if (FD_ISSET (0, &fds))
	   stdin_read ();
#endif  /* ECMD_PARSER_SUPPORT */

	if (FD_ISSET (tap_fd, &fds))
	   tap_read ();

#else
    if (newtick) {
        newtick=0;
#endif
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

#ifdef BOOTLOADER_SUPPORT
  if(bootload_delay) {
#ifdef DEBUG
    if(bootload_delay%50 == 0) {
      char seconds[5];
      debug_putstr("Time left to flash: ");
      itoa(bootload_delay/50, seconds, 10);
      debug_putstr(seconds);
      debug_putstr(" seconds\n");
    }
#endif
    if(-- bootload_delay == 0) {
      debug_putstr("RST\n");
      cli();
      _IVREG = _BV(IVCE);         /* prepare ivec change */
      _IVREG = 0x00;              /* change ivec */

#ifdef USE_WATCHDOG
      wdt_disable();
#endif
      void (*jump_to_application)(void) = NULL;
      jump_to_application();
    }
  }
#endif
  }
}
divert(-1)
timer(timer_divert_last, `counter = 0')
