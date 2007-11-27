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

#include <stdlib.h>
#include <avr/pgmspace.h>

#include "zbus_net.h"
#include "../zbus/zbus.h"
#include "../uip/uip.h"
#include "../config.h"

#ifdef ZBUS_SUPPORT

#define BUF ((struct uip_udpip_hdr *)uip_appdata-UIP_IPUDPH_LEN)

struct uip_udp_conn *zbus_conn;

void
zbus_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);

    zbus_conn = uip_udp_new(&ip, 0, zbus_net_main);

    if(! zbus_conn) 
    	return; /* dammit. */

    uip_udp_bind(zbus_conn, HTONS(ZBUS_PORT));

    zbus_core_init();
}


void
zbus_net_main(void)
{
  if (uip_newdata()) {
    if (uip_datalen() > ZBUS_BUFFER_LEN) {
      /* Send an error */
      char *p = uip_appdata;
      *p = ZBUS_UDP_ERROR_TOO_MUCH_DATA;
      uip_udp_send(1);
      return;
    }
    if (uip_udp_conn == zbus_conn) {
      /* Copy to an new connection */
      struct uip_udp_conn *tmp = uip_udp_new(&BUF->srcipaddr, 
                                             BUF->srcport,
                                             zbus_net_main);
      if (!tmp) {
        /* Send an error */
        char *p = uip_appdata;
        *p = ZBUS_UDP_ERROR_TOO_MUCH_CONNECTIONS;
        uip_udp_send(1);
        return;
      }
      uip_udp_bind(tmp, HTONS(ZBUS_PORT));
      uip_udp_conn = tmp;
      syslog_send_P(PSTR("zbus: New Connection"));
    }
    /* Copy data to the connection buffer */
    uip_udp_conn->appstate.zbus.buffer_len = uip_datalen();
    memcpy(uip_udp_conn->appstate.zbus.buffer, uip_appdata, uip_datalen());


    if (zbus_send_conn_data(uip_udp_conn) == 0) 
      uip_udp_conn->appstate.zbus.state |= ZBUS_STATE_DATA | ZBUS_STATE_SENDING;  

    /* Update the ttl of the connection */
    uip_udp_conn->appstate.zbus.ttl = 150;
    sprintf(uip_appdata, "data recieved\n");
    uip_udp_send(14);
  }
  else if (uip_poll()) {
    /* Try to send old data */
    if (uip_udp_conn->appstate.zbus.state & ZBUS_STATE_SENDING) {
      if (zbus_send_conn_data(uip_udp_conn) == 1) {
        /* Sending to the hardware layer was successfull */
        uip_udp_conn->appstate.zbus.state &= ~ZBUS_STATE_SENDING;  
        return;
      }
    }
    if (uip_udp_conn->appstate.zbus.ttl-- == 0 && uip_udp_conn != zbus_conn) {
      uip_udp_remove(uip_udp_conn);
      syslog_send_P(PSTR("zbus: Connection timeout"));
    }
  }
}

#endif /* ZBUS_SUPPORT */
