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

#include "../uip/uip.h"
#include "../config.h"
#include "../debug.h"
#include "syslog_net.h"

#ifdef SYSLOG_SUPPORT


static struct SyslogCallbackCtx syslog_callbacks[SYSLOG_CALLBACKS];


void
syslog_net_init(void)
{
  uip_ipaddr_t ip;

  CONF_SYSLOG_SERVER;

  uip_udp_conn_t *syslog_conn = uip_udp_new(&ip, SYSLOG_PORT, syslog_net_main);

  if(! syslog_conn) {
    debug_printf("syslog: couldn't create connection\n");
    return;
  }

  uip_udp_bind(syslog_conn, HTONS(SYSLOG_PORT));

  /* Set all syslog callbacks to zero */
  uint8_t i;
  for (i = 0; i < SYSLOG_CALLBACKS; i++)
    syslog_callbacks[i].callback = NULL;
}

void
syslog_net_main(void) 
{
  if (uip_poll()) {
    uint8_t i;
    for (i = 0; i < SYSLOG_CALLBACKS; i++)
      if (syslog_callbacks[i].callback != NULL) {
        syslog_callbacks[i].callback(syslog_callbacks[i].data);
        syslog_callbacks[i].callback = NULL;
        break;
      }
    
  }
}

uint8_t
syslog_insert_callback(syslog_callback_t callback, void *data)
{
  uint8_t i;
  for (i = 0; i < SYSLOG_CALLBACKS; i++)
    if (syslog_callbacks[i].callback == NULL) {
      syslog_callbacks[i].callback = callback;
      syslog_callbacks[i].data = data;
      return 1;
    }
  return 0; /* No empty callback found */
}

#endif
