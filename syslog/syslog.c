/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <avr/pgmspace.h>
#include <stdarg.h>

#include "../net/syslog_net.h"
#include "../uip/uip.h"
#include "../config.h"
#include "../debug.h"
#include "syslog.h"

#ifdef SYSLOG_SUPPORT

static char send_buffer[MAX_DYNAMIC_SYSLOG_BUFFER + 1] = "";

static void syslog_send_cb_P(void *data) 
{
  strcpy_P(uip_appdata, data);
  uip_udp_send(strlen_P(data));
}

static void syslog_send_cb(void *data) 
{
  char *p = data;

  strcpy(uip_appdata, p);
  uip_udp_send(strlen(p));

  if (p == send_buffer)
    p[0] = 0;
}


uint8_t 
syslog_send_P(PGM_P message)
{
  return syslog_insert_callback(syslog_send_cb_P, (void *)message);
}

uint8_t 
syslog_send(const char *message)
{
  // only insert a new callback if the old is finished  
  if (send_buffer[0] == 0) {
    strncpy(send_buffer, message, MAX_DYNAMIC_SYSLOG_BUFFER);
    send_buffer[MAX_DYNAMIC_SYSLOG_BUFFER] = 0;
    return syslog_insert_callback(syslog_send_cb, (void *)send_buffer);
  } else
    return 0;
}

uint8_t 
syslog_sendf(const char *message, ...)
{
  va_list va;
  va_start(va, message);

  // only insert a new callback if the old is finished  
  if (send_buffer[0] == 0) {
    vsnprintf(send_buffer, MAX_DYNAMIC_SYSLOG_BUFFER, message, va);
    send_buffer[MAX_DYNAMIC_SYSLOG_BUFFER] = 0;
    return syslog_insert_callback(syslog_send_cb, (void *)send_buffer);
  } else
    return 0;
  va_end(va);
}

uint8_t 
syslog_send_ptr(void *message)
{
  return syslog_insert_callback(syslog_send_cb, message);
}

#endif
