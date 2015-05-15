/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
 */

#include "protocols/uip/uip.h"
#include "config.h"
#include "core/debug.h"
#include "syslog_net.h"
#include "protocols/syslog/syslog.h"
#include "protocols/uip/check_cache.h"

uip_udp_conn_t *syslog_conn;

void
syslog_net_init(void)
{
  uip_ipaddr_t ip;

  set_CONF_SYSLOG_SERVER(&ip);

  syslog_conn = uip_udp_new(&ip, HTONS(SYSLOG_PORT), syslog_net_main);

  if (!syslog_conn)
  {
#ifndef DEBUG_USE_SYSLOG
    debug_printf("syslog: couldn't create connection\n");
#endif
    return;
  }

  uip_udp_bind(syslog_conn, HTONS(SYSLOG_PORT));

}

void
syslog_net_main(void)
{
  if (!uip_poll())
    return;

#ifdef ENC28J60_SUPPORT
  if (uip_check_cache(&syslog_conn->ripaddr))
    uip_slen = 1;               /* Trigger xmit to do force ARP lookup. */
#endif
}

/*
  -- Ethersex META --
  header(protocols/syslog/syslog_net.h)
  net_init(syslog_net_init)
*/
