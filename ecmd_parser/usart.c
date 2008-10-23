/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
 }}} */

#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../eeprom.h"
#include "../usart.h"
#include "ecmd.h"

#ifndef ZBUS_USE_USART
#define ZBUS_USE_USART 0 
#endif
#define USE_USART ZBUS_USE_USART 
#define BAUD CONF_ZBUS_BAUDRATE
#include "../usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

#if defined(USART_SUPPORT) && !defined(TEENSY_SUPPORT)
int16_t parse_cmd_usart_baud(char *cmd, char *output, uint16_t len)
/* {{{ */ {
    while (*cmd == ' ') cmd ++;
    if (! *cmd ) { /* No argument */
      uint16_t s_usart_baudrate;
      eeprom_restore_int(usart_baudrate, &s_usart_baudrate);
      return snprintf_P(output, len, PSTR("baudrate: %d00"), s_usart_baudrate);
    } else {
      /* Delete the last two digits */
      cmd[strlen(cmd) - 2] = 0;
      uint16_t s_usart_baudrate;
      if (sscanf_P(cmd, PSTR("%d"), &s_usart_baudrate) == 1) {
        uint16_t ubrr = usart_baudrate(s_usart_baudrate);
        usart(UBRR,H) = HI8(ubrr);
        usart(UBRR,L) = LO8(ubrr);
        eeprom_save_int(usart_baudrate, s_usart_baudrate);
        return snprintf_P(output, len, PSTR("baudrate: %d00"), s_usart_baudrate);
      } else 
        return -1;
    }
} /* }}} */
#endif
