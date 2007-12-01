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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "zbus_net.h"
#include "../zbus/zbus.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../uip/uip_neighbor.h"
#include "../network.h"
#include "../syslog/syslog.h"
#include "../config.h"

#ifdef ZBUS_SUPPORT

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

uip_udp_conn_t *zbus_conn;

void
zbus_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr_copy(&ip, all_ones_addr);

    zbus_conn = uip_udp_new(&ip, 0, zbus_net_main);

    if(! zbus_conn) 
    	return; /* dammit. */

    uip_udp_bind(zbus_conn, HTONS(ZBUS_PORT));

    zbus_core_init(zbus_conn);
}


void
zbus_net_main(void)
{
  uip_udp_conn_t error_conn;

  if (uip_newdata()) {
    if (uip_datalen() > ZBUS_BUFFER_LEN) {
      /* Send an error */
      uip_udp_send(sprintf_P(uip_appdata, PSTR("EToo much data")));
      goto send_answer;
    }
    if (uip_udp_conn == zbus_conn) {
      /* Copy to an new connection */
      uip_udp_conn_t *tmp = uip_udp_new(&BUF->srcipaddr, 
                                        BUF->srcport,
                                        zbus_net_main);
      uip_udp_bind(tmp, HTONS(ZBUS_PORT));
      if (!tmp) {
        uip_udp_send(sprintf_P(uip_appdata, PSTR("EToo much connections")));
        goto send_answer;
      }
      uip_udp_bind(tmp, HTONS(ZBUS_PORT));
      uip_udp_conn = tmp;
    }
    /* Copy data to the connection buffer */
    uip_udp_conn->appstate.zbus.buffer_len = uip_datalen();
    memcpy(uip_udp_conn->appstate.zbus.buffer, uip_appdata, uip_datalen());


    if (zbus_send_conn_data(uip_udp_conn) == 0) 
      uip_udp_conn->appstate.zbus.state |= ZBUS_STATE_SENDING;  

    /* Update the ttl of the connection */
    uip_udp_conn->appstate.zbus.ttl = 25;
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
    /* Do timeout for the single zbus connections */
    if (uip_udp_conn->appstate.zbus.ttl-- == 0 && uip_udp_conn != zbus_conn) 
      uip_udp_remove(uip_udp_conn);

    /* Poll the hardware subsystem */
    if (uip_udp_conn == zbus_conn)
      zbus_core_periodic();

    if (uip_udp_conn == zbus_conn 
        && uip_udp_conn->appstate.zbus.state & ZBUS_STATE_RECIEVED) {
      /* New data arrived 
       * Send the recieved data to the connection, which sent the last time
       * to the address ( always the first byte )
       */
      for(uip_udp_conn = &uip_udp_conns[0];
          uip_udp_conn < &uip_udp_conns[UIP_UDP_CONNS];
          ++uip_udp_conn) {
        if (uip_udp_conn->callback == zbus_net_main
            && uip_udp_conn->lport
            && (uip_udp_conn->appstate.zbus.buffer[1]  
                == zbus_conn->appstate.zbus.buffer[0])
            && uip_udp_conn != zbus_conn) {
          /* Success udp messages always begins with O */
          ((unsigned char *)uip_appdata)[0] = 'O';

          memcpy(uip_appdata + 1, zbus_conn->appstate.zbus.buffer, 
                 zbus_conn->appstate.zbus.buffer_len + 1);
          uip_udp_send(zbus_conn->appstate.zbus.buffer_len + 1);
          zbus_conn->appstate.zbus.state &= ~ZBUS_STATE_RECIEVED;
          return;
        }
      }
      zbus_conn->appstate.zbus.state &= ~ZBUS_STATE_RECIEVED;
    }
  }
  return;

send_answer:
  /* Send an error */
  uip_ipaddr_copy(error_conn.ripaddr, BUF->srcipaddr);
  error_conn.rport = BUF->srcport;
  error_conn.lport = HTONS(ZBUS_PORT);
  error_conn.ttl = uip_udp_conn->ttl;

  uip_udp_conn = &error_conn;

  /* Send immediately */
  uip_process(UIP_UDP_SEND_CONN); 
#ifdef IPV6_SUPPORT
  uip_neighbour_out();
#else
  uip_arp_out(); 
#endif
  transmit_packet();

  uip_slen = 0;

  uip_udp_conn = zbus_conn;
}

#endif /* ZBUS_SUPPORT */
