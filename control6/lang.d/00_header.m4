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
# M4 Macro rename
define(`m4_macro_rename', `define(`$2', defn(`$1'))undefine(`$1')')
# Counters
define(`action_thread_count', 0)
define(`ecmd_global_count', 0)
define(`timer_count', 0)
define(`pin_count', 0)

# different levels of output
define(`action_table_divert', 0)
define(`timer_divert', 1)
define(`globals_divert', 2)
define(`pin_table_divert', 3)
define(`ecmd_variable_divert', 4)
define(`action_divert', 5)
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
#include "services/clock/clock.h"
#include "core/pt/pt.h"
#include "config.h"
#include "hardware/adc/kty/kty81.h"
#ifdef NAMED_PIN_SUPPORT
#include "core/portio/user_config.h"
#endif
#include "protocols/ecmd/sender/ecmd_sender_net.h"
#include "protocols/uip/uip.h"
#include "control6.h"

divert(-1)dnl
define(`C6_HEADER', `')

