dnl
dnl   Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
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

# foreach((item_1, item_2, ..., item_n), stmt)
# foreach(((arg_1, arg_2, ..., arg_n), (arg_1, arg_2, ..., arg_n), ..., (...)), stmt)
# in stmt $1, $2, ..., $n 
define(`_arg1', `$1')
define(`FOREACH', `ifelse(`$1', `()', `',dnl
`define(`_foreach_', `$2')define(`_arg', _arg1$1)dnl
_foreach_(regexp(_arg, `^(?\(.*\))?$', `\1'))`'$0((shift$1), `$2')')')


# M4 Macro rename
define(`m4_macro_rename', `define(`$2', defn(`$1'))undefine(`$1')')
# Counters
define(`action_thread_count', 0)
define(`ecmd_global_count', 0)
define(`timer_count', 0)
define(`pin_count', 0)

# different levels of output
define(`action_table_divert', 0)divert(action_table_divert)/* C6-DIVERT: action_table_divert */
define(`timer_divert', 1)divert(timer_divert)/* C6-DIVERT: timer_divert */
define(`globals_divert', 2)divert(globals_divert)/* C6-DIVERT: globals_divert */
define(`pin_table_divert', 3)divert(pin_table_divert)/* C6-DIVERT: pin_table_divert */
define(`ecmd_variable_divert', 4)divert(ecmd_variable_divert)/* C6-DIVERT: ecmd_variable_divert */
define(`action_divert', 5)divert(action_divert)/* C6-DIVERT: action_divert */
define(`init_divert', 9)divert(init_divert)/* C6-DIVERT: init_divert */
define(`normal_start_divert', 10)divert(normal_start_divert)/* C6-DIVERT: normal_start_divert */
define(`normal_divert', 11)divert(normal_divert)/* C6-DIVERT: normal_divert */
define(`normal_end_divert', 12)divert(normal_end_divert)/* C6-DIVERT: normal_end_divert */
define(`update_pin_divert', 13)divert(update_pin_divert)/* C6-DIVERT: update_pin_divert */
define(`control_end_divert', 14)divert(control_end_divert)/* C6-DIVERT: control_end_divert */

divert(-1)dnl
################################
# Headers
################################
divert(0)dnl
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "services/clock/clock.h"
#include "core/pt/pt.h"
#include "core/vfs/vfs.h"
#include "core/vfs/vfs-util.h"
#include "config.h"
#include "hardware/adc/kty/kty81.h"
#ifdef NAMED_PIN_SUPPORT
#include "core/portio/user_config.h"
#endif
#include "protocols/uip/uip_router.h"
#include "protocols/uip/uip.h"
#include "control6.h"

divert(timer_divert)uint32_t timers[] = {
divert(-1)dnl
define(`C6_HEADER', `')

