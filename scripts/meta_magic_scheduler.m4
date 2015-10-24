dnl
dnl   Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl   Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
dnl   Copyright (c) 2013-2015 by Michael Brakemeier <michael@brakemeier.de>
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
dnl -------------------------
divert(-1)dnl
dnl debugfile(`meta_magic_scheduler.trace')dnl
dnl debugmode(`V')dnl

dnl
dnl some useful m4 macros
dnl
dnl ifndef - reverse built-in ifdef macro
define(`ifndef', `ifdef(`$1', `$3', `$2')')dnl

dnl array_set(name, index, value)
define(`array_set', `define(`$1[$2]', `$3')')dnl
dnl array_get(name, index)
define(`array_get', `defn(`$1[$2]')')dnl
dnl array_append(name, value) -- append element value
define(`array_append', `array_set($1, size, incr(array_get($1, size)))`'dnl
array_set($1, array_get($1, size), `$2')')dnl
dnl array_new(name [, size, initial]) -- initialize array [opt. with size
dnl elements with values initial]
define(`array_new', `array_set($1, size, 0)ifelse(eval(`$# > 1'), `1',dnl
`forloop(`x', 1, $2, `array_append(`$1', `$3')')')')dnl
dnl  array_swap(name, j, name[j], i)
define(`array_swap',dnl
`array_set($1, $2, array_get($1, $4))`'array_set($1, $4, `$3')')dnl

dnl bubblesort(name)
dnl bubblesort an array with user supplied compare macro
dnl requires compare macro bsortkey(name, index) to be defined
define(`bubbleonce',
`forloop(`x', 1, $2,dnl
`ifelse(eval(bsortkey($1, x) > bsortkey($1, incr(x))), 1,dnl
`array_swap($1, x, array_get($1, x), incr(x))`'1')')0')dnl
define(`bubbleupto',
`ifelse(bubbleonce($1, $2), 0, `', `bubbleupto($1, decr($2))')')dnl
define(`bubblesort',
`bubbleupto($1, decr(array_get($1,size)))')dnl

dnl from m4-1.4.16/examples/forloop3.m4:
dnl forloop_arg(from, to, macro) - invoke MACRO(value) for
dnl   each value between FROM and TO, without define overhead
define(`forloop_arg', `ifelse(eval(`($1) <= ($2)'), `1',dnl
`_forloop(`$1', eval(`$2'), `$3(', `)')')')dnl
dnl forloop(var, from, to, stmt) - refactored to share code
define(`forloop', `ifelse(eval(`($2) <= ($3)'), `1',dnl
`pushdef(`$1')_forloop(eval(`$2'), eval(`$3'),dnl
`define(`$1',', `)$4')popdef(`$1')')')dnl
define(`_forloop',dnl
`$3`$1'$4`'ifelse(`$1', `$2', `',dnl
`$0(incr(`$1'), `$2', `$3', `$4')')')dnl

dnl for(var, from, to, step, stmt)
define(`for', `ifelse($#,0,``$0'',
`ifelse(eval(`($2) <= ($3)'), `1',
`pushdef(`$1',$2)$5`'popdef(`$1')$0(`$1',eval($2+$4),$3,$4,`$5')')')')dnl

dnl save current divert value
define(`pushdivert', `define(`_old_divert', divnum)')dnl
define(`popdivert', `divert(_old_divert)')dnl

dnl Errors and warnings
define(`error_msg', `ifelse($#, 0, ``$0'', $1,,,dnl
`errprint(ifdef(`__program__', `__program__', ``m4'')'dnl
`:ifelse(__line__, `0', `',
`__file__:__line__:')` ERROR: $*
')m4exit(`1')')')dnl

define(`warning_msg', `ifelse($#, 0, ``$0'', $1,,,dnl
`errprint(ifdef(`__program__', `__program__', ``m4'')'dnl
`:ifelse(__line__, `0', `',
`__file__:__line__:')` WARNING: $*
')')')dnl

dnl strip trailing Brackets from function names
define(`stripBrackets', `patsubst($*, `(.*)$')')dnl

dnl diverts used in this script
define(`prototypes', 0)dnl
dnl NASTY DRAGON INSIDE! m4-scripts for ecmd, SOAP, etc. use diverts from 1 up to 11(?)
define(`timer_divert', `eval(prototypes` + 12')')dnl
define(`timer_divert_static_start', `eval(timer_divert` + 1')')dnl
define(`timer_divert_static_end', `eval(timer_divert_static_start` + 1')')dnl
define(`timer_divert_static_control_start', `eval(timer_divert_static_end` + 1')')dnl
define(`timer_divert_static_control_end', `eval(timer_divert_static_control_start` + 1')')dnl
define(`implementation_start_divert', `eval(timer_divert_static_control_end` + 1')')dnl
define(`initearly_divert', `eval(implementation_start_divert` + 1')')dnl
define(`init_divert', `eval(initearly_divert` + 1')')dnl
define(`net_init_divert', `eval(init_divert` + 1')')dnl
define(`exit_divert', `eval(net_init_divert` + 1')')dnl
define(`startup_divert', `eval(exit_divert` + 1')')dnl
define(`mainloop_divert', `eval(startup_divert` + 1')')dnl
define(`periodic_process_divert', `eval(mainloop_divert` + 1')')dnl
define(`postamble_divert', `eval(periodic_process_divert` + 1')')dnl

dnl timer(n, func)
dnl old-style timer(n, func) macros are rewritten to
dnl millitimer(i, func) macros with i = n * 20 ms intervals
define(`timer', `ifelse($#, 0, ``$0'', $1,,,dnl
`millitimer(eval(`($1) * 20'), `$2')')')dnl

dnl ms2counts(interval)
dnl calculate number of milliticker counts for given interval in ms
define(`ms2counts', `ifelse($#, 0, ``$0'', $1,,,dnl
`dnl error: millitimer interval requires higher tickrate
ifelse(eval((`($1) * 'value_MTICKS_PER_SEC)`/1000 < 1'), `1',dnl
`error_msg(`Periodic ticks: at least one millitimer requires higher tickrate.')')dnl
dnl warn: millitimer interval will be rounded (is prime or something like that)
ifelse(eval((`($1) * 'value_MTICKS_PER_SEC)`%1000 != 0'), `1',dnl
`warning_msg(`Periodic ticks: at least one millitimer interval will be adjusted.')')dnl
eval((`($1) * 'value_MTICKS_PER_SEC)`/1000')dnl
')')dnl

dnl initialize m4 millitimer array
array_new(`millitimer_array')

dnl millitimer(interval, func)
dnl millitimer() is similar to timer with the exception that the
dnl first arg specifies the number of milliseconds between invocations
dnl of the second arg
define(`millitimer',`ifelse($#, 0, ``$0'', $1,,,dnl
`array_append(`millitimer_array', format(`%d/%s', ms2counts($1), stripBrackets(`$2')))
')')dnl

dnl
dnl sort and distribute static millitimers
dnl
dnl slots in profiling array
define(`_num_time_slots', eval(value_MTICKS_PER_SEC * 3 / 2))

dnl initialize profiling array
array_new(`profile_array', _num_time_slots, `0')

dnl millitimer_func(index)
define(`millitimer_func',
`substr(array_get(`millitimer_array', $1), eval(index(array_get(`millitimer_array', $1), `/')`+ 1'))')dnl

dnl millitimer_ival(index)
define(`millitimer_ival',
`substr(array_get(`millitimer_array', $1), 0, index(array_get(`millitimer_array', $1), `/'))')dnl

dnl bubble sort compare macro
define(`bsortkey', `millitimer_ival($2)')dnl

dnl print_millitimer(func, startval, ival)
dnl return a formatted static timer C array entry
define(`print_millitimer',
`pushdivert()divert(timer_divert_static_start)dnl
format(`    { %s, %d }, /* static timer */
', `$1', `$3')popdivert()dnl
pushdivert()divert(timer_divert_static_control_start)dnl
format(`    { %d, TIMER_RUNNABLE }, /* %s() static timer control */
', `$2', `$1')popdivert()')

dnl statictimer()
dnl sort and distribute static timers
define(`statictimer', dnl `ifelse($#, 0, ``$0'', $1,,,dnl
dnl -- distribution results are better with a sorted timer array as input
`bubblesort(`millitimer_array')dnl
dnl -- timer distribution losely based on greedy
forloop(`elem', 1, array_get(`millitimer_array', size),
`ifelse(eval(millitimer_ival(elem) == `1'), `1',
`dnl -- the easy case, call timer on every cycle
forloop(`iter', 1, _num_time_slots, `array_set(`profile_array',iter, incr(array_get(`profile_array',iter)))')
print_millitimer(millitimer_func(elem), `1', `1')',
eval(millitimer_ival(elem) > value_MTICKS_PER_SEC), `1', dnl  > 1 SEKUNDE
`dnl -- timer > 1s, simply pick a rarely used timer slot
define(`_startval', `7')
forloop(`iter', `1', _num_time_slots,
`ifelse(eval(array_get(`profile_array',iter) < array_get(`profile_array',_startval)), `1',
`ifelse(eval(array_get(`profile_array',iter) <= array_get(`profile_array',eval(iter` + 1'))), `1',
`define(`_startval', iter)')')')
array_set(`profile_array', _startval, incr(array_get(`profile_array',_startval)))
print_millitimer(millitimer_func(elem), eval(millitimer_ival(elem) + _startval), millitimer_ival(elem))',
`true', `true',
`dnl -- catch all others, distribute by profiling
define(`_startval', millitimer_ival(elem))
define(`_topval', `ifelse(eval(millitimer_ival(elem) * 2 < _num_time_slots), `1',
`eval(millitimer_ival(elem) * 2)', _num_time_slots)')
forloop(`iter', _startval, _topval,
`ifelse(eval(array_get(`profile_array',iter) < array_get(`profile_array',_startval)), `1',
`ifelse(eval(array_get(`profile_array',iter) <= array_get(`profile_array',eval(iter` + 1'))), `1',
`define(`_startval', iter)')')')
for(`iter', _startval, _num_time_slots, millitimer_ival(elem), `array_set(`profile_array',iter, incr(array_get(`profile_array',iter)))')
print_millitimer(millitimer_func(elem), _startval, millitimer_ival(elem))')')
dnl debugging, dump contents of profile array
dnl ifelse(`dnl comment to enable debugging output
pushdivert()divert(timer_divert_static_start)dnl
forloop(`iter', 1, array_get(`profile_array', size),dnl
`format(`    // slot %4d - (%d): ', iter, array_get(`profile_array', iter))dnl
forloop(`chars', 1, array_get(`profile_array', iter), `+')
')popdivert()dnl
dnl ')dnl comment to enable debugging output
')dnl


dnl forloop(`iter', 1, array_get(`profile_array', size), `
dnl pushdivert()divert(postamble_divert)dnl
dnl format(`// %4d - (%d): ', iter, array_get(`profile_array',iter))dnl
dnl forloop(`chars', 1, array_get(`profile_array',iter), `+')popdivert()')')

dnl divert`'dnl
divert(prototypes)dnl
dnl *** Start of meta.c contents ***
/*
 * This file has been generated automatically by meta_magic_scheduler.m4.
 * Please do not modify it, edit the m4 scripts instead.
 */

#include "config.h"

#if ARCH != ARCH_HOST
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#endif

#include <stdint.h>
#include "core/debug.h"

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

#endif /* ARCH == ARCH_HOST */

void ethersex_meta_init(void);
void ethersex_meta_netinit(void);
void ethersex_meta_startup(void);
void ethersex_meta_mainloop(void);
void dyndns_update(void);
void periodic_process(void);

volatile uint8_t newtick;

dnl
dnl Timer foo
dnl
divert(timer_divert)
/*
 * All the static millitimer functions invoked by the scheduler.
 */
const static_timer_func_t PROGMEM scheduler_static_timers[] = {
 /* { timer_t timer, uint16_t interval } */
divert(timer_divert_static_start)dnl
divert(timer_divert_static_end)dnl
};

/*
 * Static millitimers control block in RAM.
 */
static_timer_cb_t scheduler_static_timers_control[] = {
 /* { uint16_t delay, uint8_t state } */
divert(timer_divert_static_control_start)dnl
divert(timer_divert_static_control_end)dnl
};

/*
 * Actual number of millitimer entries in array is only known here, propagate.
 */
const uint8_t scheduler_static_timer_max = sizeof(scheduler_static_timers) / sizeof(scheduler_static_timers[0]);

dnl
dnl functions follow
dnl
divert(implementation_start_divert)
/*
 * All the ethersex meta functions.
 */
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

divert(periodic_process_divert)dnl
    periodic_process(); wdt_kick();

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

dnl last divert
divert(postamble_divert)
/*
 * periodic_process() does the real work.
 */
void periodic_process(void)
{
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

#else /* not ARCH_HOST */
    if (newtick) {
      newtick=0;
#endif /* ARCH == ARCH_HOST */

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
#endif /* RFM12_IP_SUPPORT */
      }
#endif /* UIP_SUPPORT */
      /* call the schedulers dispatch function */
      scheduler_dispatch_timer();

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
      } /* if (bootload_delay) */
#endif /* BOOTLOADER_SUPPORT */
    } /* if (newtick) */
  }
divert(-1)dnl

dnl
dnl postprocess static timers after all files have been read
m4wrap(`statictimer')

