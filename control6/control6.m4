dnl
dnl   Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
dnl   Copyright (c) 2008,2009 by Jochen Roessner <jochen@lugrot.de>
dnl   Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl  
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 2 or later
dnl   as published by the Free Software Foundation.
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
divert(-1)
# M4 Macro rename
define(`m4_macro_rename', `define(`$2', defn(`$1'))undefine(`$1')')
# Counters
define(`action_thread_count', 0)
define(`timer_count', 0)
define(`pin_count', 0)

# different levels of output
define(`action_table_divert', 0)
define(`timer_divert', 1)
define(`globals_divert', 2)
define(`pin_table_divert', 3)
define(`action_divert', 4)
define(`init_divert', 9)
define(`normal_start_divert', 10)
define(`normal_divert', 11)
define(`normal_end_divert', 12)
define(`update_pin_divert', 13)
define(`control_end_divert', 14)

################################
# Headers
################################
divert(0)dnl
#include <avr/io.h>
#include "../clock/clock.h"
#include "../pt/pt.h"
#include "../config.h"
#include "../kty/kty81.h"
#ifdef NAMED_PIN_SUPPORT
#include "../named_pin/user_config.h"
#endif
#include "../net/ecmd_sender_net.h"
#include "../bit-macros.h"
#include "../uip/uip.h"

divert(-1)dnl

################################
# Global Variables
################################

define(`GLOBAL', `define(`old_divert', divnum)dnl
divert(globals_divert)ifelse(`$#', 2, `$2', `uint8') $1;
divert(old_divert)')

################################
# Actions
################################
define(`THREAD', `define(action_thread_ident, '__line__`)divert(0)dnl
 {0, {0} },define(`action_thread_$1_idx', action_thread_count)dnl
define(`action_thread_count', incr(action_thread_count))dnl
divert(action_divert)dnl

static 
PT_THREAD(action_thread_$1(struct pt *pt)) {
  PT_BEGIN(pt);
divert(normal_end_divert)
  if (action_threads[action_thread_$1_idx].started) { THREAD_DO($1) }dnl
divert(action_divert)')

define(`INTHREAD', `ifdef(`action_thread_ident', `$1', `$2')')
define(`DIE', `errprint(`ERROR: $1
')m4exit(255)')

define(`THREAD_END', `divert(action_divert)dnl
undefine(`action_thread_ident')dnl
dnl PT_WAIT_WHILE(pt, 1);
  PT_END(pt);
}

divert(normal_divert)dnl')
define(`THREAD_DO', `action_thread_$1(&action_threads[action_thread_$1_idx].pt);')
define(`THREAD_START',  `action_threads[action_thread_$1_idx].started = 1;')
define(`THREAD_STOP',  `action_threads[action_thread_$1_idx].started = 0;')
define(`THREAD_WAIT',  `action_threads[action_thread_$1_idx].started = 0;
PT_WAIT_WHILE(pt, action_threads[action_thread_$1_idx].started == 1);')
define(`THREAD_RESTART',  `do { action_threads[action_thread_$1_idx].started = 1; 
  PT_INIT(&action_threads[action_thread_$1_idx].pt); } while(0);')
divert(action_table_divert)dnl
struct action {
  uint8_t started;
  struct pt pt;
};

define(`THREAD_STARTED', `action_threads[action_thread_$1_idx].started')

divert(-1)
################################
# Control
################################
define(`CONTROL_START', `divert(action_table_divert)struct action action_threads[] = {
divert(timer_divert)uint32_t timers[] = {
divert(init_divert)void control6_init(void) {
divert(normal_start_divert)void control6_run(void) { dnl
divert(normal_divert)')
define(`CONTROL_END', `divert(control_end_divert)
}
divert(timer_divert)
};

divert(pin_table_divert)
};

divert(init_divert)dnl
}

divert(action_table_divert)
};')

################################
# Timers
################################
define(`TIMER_NEW', `ifdef(`timer_$1', `', `dnl
define(`old_divert', divnum)
divert(timer_divert) -1,dnl
divert(old_divert)dnl
define(`timer_$1', timer_count)dnl
define(`timer_count', incr(timer_count))')')
define(`TIMER_START', `TIMER_NEW($1)  timers[timer_$1] = clock_get_time();')

define(`TIMER', `ifdef(`timer_used', `', `dnl
define(`old_divert', divnum)dnl
divert(globals_divert)uint32_t act_time;
#ifndef CLOCK_SUPPORT
#error Please define clock support
#endif

divert(normal_start_divert)act_time = clock_get_time();
define(`timer_used')dnl
divert(old_divert)')dnl
(act_time - timers[timer_$1])')

define(`TIMER_WAIT', `PT_WAIT_UNTIL(pt, TIMER($1) >= $2);')
define(`WAIT', `TIMER_START(`timer_on_'__line__); TIMER_WAIT(`timer_on_'__line__, ($1));')

################################
# Conditionals
################################
define(`ON', `if (')
define(`DO', `ifelse(`$#', 0, `) {', `THREAD($1)divert(normal_divert)) { THREAD_START($1) }divert(action_divert)')')
define(`END', `ifelse(`$#', 0, `}', `THREAD_END($1)}')')
define(`BETWEEN', `$1 > $2 && $1 < $3')
define(`NOT', `ifelse(`$#', 0, `!', `! ( $1 )')')

################################
# Pins
################################
define(`PIN_INPUT', `define(`old_divert', divnum)divert(init_divert)  DDR_CONFIG_IN($1);
divert(old_divert)dnl')
define(`PIN_OUTPUT', `define(`old_divert', divnum)divert(init_divert)  DDR_CONFIG_OUT($1);
divert(old_divert)dnl')
define(`PIN_PULLUP', `define(`old_divert', divnum)divert(init_divert)  PIN_SET($1);
divert(old_divert)dnl')

define(`PIN_NEW', `ifdef(`pin_$1_used', `', `define(`pin_$1_used')define(`old_divert', divnum)dnl
divert(pin_table_divert)  { 0 },dnl
define(`pin_$1_idx', pin_count)dnl
define(`pin_count', incr(pin_count))dnl
divert(init_divert) pin_states[pin_$1_idx].old_state = PIN_HIGH($1) ? 1 : 0; 
divert(update_pin_divert)  
  pin_states[pin_$1_idx].old_state = PIN_HIGH($1) ? 1 : 0; 
divert(old_divert)')')

define(`PIN_RISING', `PIN_NEW($1)(!pin_states[pin_$1_idx].old_state && PIN_HIGH($1))')
define(`PIN_FALLING', `PIN_NEW($1)(pin_states[pin_$1_idx].old_state && !PIN_HIGH($1))')
define(`PIN_LOW', `!PIN_HIGH($1)')

divert(pin_table_divert)
struct pin_state {
  uint8_t old_state;
};

struct pin_state pin_states[] ={
divert(-1)

################################
# CLOCK
################################
define(`CLOCK_USED', `ifdef(`clock_used', `', `dnl
define(`old_divert', divnum)dnl
define(`clock_used')dnl
divert(globals_divert)struct clock_datetime_t datetime;
uint8_t last_minute;
#ifndef CLOCK_SUPPORT
#error Please define clock support
#endif

divert(normal_start_divert)  clock_localtime(&datetime, clock_get_time());
divert(control_end_divert)  last_minute = datetime.min;
divert(old_divert)')')

define(`CLOCK_MIN', `CLOCK_USED()(datetime.min != last_minute) && datetime.min')
define(`CLOCK_HOUR', `CLOCK_USED()datetime.hour')
define(`CLOCK_DAY', `CLOCK_USED()datetime.day')
define(`CLOCK_MONTH', `CLOCK_USED()datetime.month')
define(`CLOCK_DOW', `CLOCK_USED()datetime.dow')
define(`CLOCK_YEAR', `CLOCK_USED()datetime.yead')

################################
# ADC
################################
define(`ADC_USED', `ifdef(`adc_used', `', `dnl
define(`old_divert', divnum)dnl
define(`adc_used')dnl
divert(globals_divert)
#ifndef ADC_SUPPORT
#error Please define adc support
#endif
static uint16_t
control6_get_adc(uint8_t sensorchannel){
  ADMUX = (ADMUX & 0xF0) | sensorchannel;
  /* Start der adc konvertierung */
  ADCSRA |= _BV(ADSC);
  /* Warten bis sie fertig ist */
  while (ADCSRA & _BV(ADSC)) {}
  return ADC;
}

divert(old_divert)')')

define(`ADC_GET', `ADC_USED()control6_get_adc($1)')

################################
# KTY81
################################
define(`KTY_USED', `ifdef(`kty_used', `', `dnl
define(`old_divert', divnum)dnl
define(`kty_used')dnl
divert(globals_divert)
#ifndef KTY_SUPPORT
#error Please define kty support
#endif

divert(old_divert)')')

define(`KTY_GET', `KTY_USED()temperatur(get_kty($1))')

################################
# ECMD SENDER
################################
define(`ECMD_SENDER_USED', `ifdef(`ecmd_sender_used', `', `dnl
define(`old_divert', divnum)dnl
define(`ecmd_sender_used')dnl
divert(globals_divert)
#ifndef ECMD_SENDER_SUPPORT
#error Please define emcd sender tcp support
#endif

divert(old_divert)')')

define(`IPADDR', `ifelse(regexp($1, `:'), `-1', `ip4addr_expand(translit(`$1', `.', `,'))', 
	`ip6addr_expand(regexp(`$1', `\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\)',`0x\1,0x\2,0x\3,0x\4,0x\5,0x\6,0x\7,0x\8')) ') ')

define(`ip4addr_expand', `HTONS(($1 << 8) | $2), HTONS(($3 << 8) | $4)')
define(`ip6addr_expand', `uip_ip6addr_t ip; uip_ip6addr(&ip, HTONS($1),  HTONS($2),  HTONS($3),  HTONS($4),  
 	 HTONS($5),  HTONS($6),  HTONS($7), HTONS($8))')

define(`ESEND', `{IPADDR($1);ecmd_sender_send_command(&ip, PSTR($2), NULL); }')

################################
# UECMD SENDER
################################
define(`UECMD_SENDER_USED', `ifdef(`uecmd_sender_used', `', `dnl
define(`old_divert', divnum)dnl
define(`uecmd_sender_used')dnl
divert(globals_divert)
#ifndef UECMD_SENDER_SUPPORT
#error Please define emcd sender udp support
#endif

#include "../net/ecmd_state.h"

divert(old_divert)')')

define(`IPADDR', `ifelse(regexp($1, `:'), `-1', `ip4addr_expand(translit(`$1', `.', `,'))', 
	`ip6addr_expand(regexp(`$1', `\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\)',`0x\1,0x\2,0x\3,0x\4,0x\5,0x\6,0x\7,0x\8')) ') ')

define(`ip4addr_expand', `HTONS(($1 << 8) | $2), HTONS(($3 << 8) | $4)')
define(`ip6addr_expand', `uip_ip6addr_t ip; uip_ip6addr(&ip, $1, $2, $3, $4, $5, $6, $7, $8)')

define(`UESEND', `UECMD_SENDER_USED(){IPADDR($1);uecmd_sender_send_command(&ip, PSTR($2), NULL); }')

define(`UESENDGET', `INTHREAD(`', `DIE(`Can use UESENDGET only in a THREAD')')UECMD_SENDER_USED(){IPADDR($1);
`uecmd_callback_blocking'action_thread_ident` = 1; 
uecmd_sender_send_command(&ip, PSTR($2), uecmd_callback'action_thread_ident`); 
PT_WAIT_WHILE(pt, uecmd_callback_blocking'action_thread_ident` == 1);' }
ifdef(`uecmd_callback_defined'action_thread_ident, `', `
define(`uecmd_callback_defined'action_thread_ident, 1)
define(`old_divert', divnum)dnl
divert(globals_divert)dnl
uint8_t uecmd_callback_blocking'action_thread_ident`;
uint8_t uecmd_callback_buffer'action_thread_ident`[ECMD_INPUTBUF_LENGTH];
uint8_t uecmd_callback_buffer_len'action_thread_ident`;

void uecmd_callback'action_thread_ident`(char *text, uint8_t len) {
  uecmd_callback_blocking'action_thread_ident` = 0;
  uecmd_callback_buffer_len'action_thread_ident` = len;
  if (text) {
    memset(uecmd_callback_buffer'action_thread_ident`, 0, ECMD_INPUTBUF_LENGTH);
    memcpy(uecmd_callback_buffer'action_thread_ident`, text, 
           (ECMD_INPUTBUF_LENGTH - 1) < len ? ECMD_INPUTBUF_LENGTH - 1 : len);
    
  }
}')

divert(old_divert)')')

define(`UESENDGET_BUFFER', `INTHREAD(`', `DIE(`Can use USENDGET_BUFFER only in a THREAD')')`uecmd_callback_buffer'action_thread_ident')
define(`UESENDGET_BUFFER_LEN', `INTHREAD(`', `DIE(`Can use USENDGET_BUFFER_LEN only in a THREAD')')`uecmd_callback_buffer_len'action_thread_ident')

###############################
# Global flags
###############################
define(`STARTUP', `ifdef(`startup_used', `', `dnl
define(`old_divert', divnum)dnl
define(`startup_used')dnl
divert(globals_divert)static uint8_t global_started = 0;
divert(control_end_divert)

  global_started = 1;dnl
divert(old_divert)')global_started == 0')

ifelse(LANGUAGE, `GERMAN', `include(`german.m4')')

divert(0)dnl

