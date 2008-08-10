/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../config.h"
#include "../net/mcuf_net.h"
#include "../syslog/syslog.h"
#include "mcuf.h"
#include "../uip/uip.h"

#include "../pinning.c"

#ifdef MCUF_SUPPORT

#define USE_USART MCUF_USE_USART
#include "../usart.h"


/* We generate our own usart init module, for our usart port */
generate_usart_init(MCUF_UART_UBRR)


void
mcuf_init(void)
{
    /* Initialize the usart module */
    usart_init();
    /* Disable the receiver */
    usart(UCSR,B) &= ~_BV(usart(RXCIE));
}

void
mcuf_newdata(void) 
{
  /* MCUF Magic bytes */
  if ( strncmp(uip_appdata, "\x23\x54\x26\x66", 4) == 0) {

  }
}

#endif
