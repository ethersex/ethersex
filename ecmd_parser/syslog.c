/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Thorsten Schroeder <ths@dev.io>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_arp.h"
#include "core/bit-macros.h"
#include "core/eeprom.h"
#include "../syslog/syslog.h"
#include "ecmd.h"

#ifdef SYSLOG_SUPPORT

#ifndef TEENSY_SUPPORT
extern int16_t print_ipaddr (uip_ipaddr_t *addr, char *output, uint16_t len);
extern int8_t parse_ip(char *cmd, uip_ipaddr_t *ptr);
#endif


int16_t parse_cmd_show_syslog(char *cmd, char *output, uint16_t len)
{
    return print_ipaddr (syslog_getserver (), output, len);
}

int16_t parse_cmd_syslog (char *cmd, char *output, uint16_t len)
{
    syslog_send(cmd);
    return 0;
}

#endif /* SYSLOG_SUPPORT */
