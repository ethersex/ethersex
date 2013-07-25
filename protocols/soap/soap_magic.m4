dnl This m4 script uses quite a few divert levels, these are essentially:
dnl   1: function prototypes
dnl   2: char array in program space
dnl   3: ----
dnl   4: function list trailer
dnl
dnl ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
divert(0)dnl
/* This file has been generated with the generous help of soap_magic.m4 */

#include "protocols/soap/soap.h"

divert(4)dnl

/* Soap function name <-> soap rpc name mapping */
const soap_command_t PROGMEM soap_cmds[] = {
divert(-1)dnl

define(`soap_rpc', `dnl
divert(1)uint8_t $1 (uint8_t, soap_data_t *args, soap_data_t *result);
divert(2)const char PROGMEM soap_$1_text[] = $2;
divert(4)	{ soap_$1_text, $1 },
divert(-1)')

divert(5)dnl
        { NULL, NULL }
};
divert(99)/* Heil Eris! */
divert(-1)dnl
dnl yippie, we're done!
