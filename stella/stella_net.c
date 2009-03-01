/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
#include "../stella/stella.h"
#include "stella_net.h"

#ifdef STELLA_SUPPORT


void
stella_net_init(void)
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  uip_udp_conn_t *stella_conn = uip_udp_new(&ip, 0, stella_net_main);

  if(! stella_conn) {
    debug_printf("syslog: couldn't create connection\n");
    return;
  }

  uip_udp_bind (stella_conn, HTONS(STELLA_UDP_PORT));
  stella_pwm_init ();

#if DEBUG_STELLA
  debug_printf("Stella initalized\n");
#endif
}

void
stella_net_main(void) 
{
  if (!uip_newdata ())
    return;

#if DEBUG_STELLA
  debug_printf("Received stella package\n");
#endif

  stella_process (uip_appdata, uip_len);
}

#endif /* STELLA_SUPPORT */
