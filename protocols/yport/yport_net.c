/*
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "yport_net.h"
#include "uip/uip.h"
#include "debug.h"
#include "yport.h"

#include "config.h"

#ifdef YPORT_SUPPORT

uip_conn_t *yport_conn = NULL;

void yport_net_init(void)
{
  uip_listen(HTONS(YPORT_PORT), yport_net_main);
}

void yport_net_main(void)
{
  if(uip_connected()) {
    if (yport_conn == NULL) {
      yport_conn = uip_conn;
      uip_conn->wnd = YPORT_BUFFER_LEN - 1;
    }
    else
      /* if we have already an connection, send an error */
      uip_send("ERROR: Connection blocked\n", 27);
  } else if (uip_acked()) {
    /* If the peer is not our connection, close it */
    if (yport_conn != uip_conn)
      uip_close();
    else {
      /* Some data we have sent was acked, jipphie */
      /* disable interrupts */
      uint8_t sreg = SREG; cli();
      yport_recv_buffer.len -= yport_recv_buffer.sent;
      /* We should use memmove, because the data may overlap */
      memmove(yport_recv_buffer.data,
              yport_recv_buffer.data + yport_recv_buffer.sent,
              yport_recv_buffer.len);
      /* enable interrupts again */
      SREG = sreg;
    }
  } else if (uip_closed() || uip_aborted() || uip_timedout()) {
    /* if the closed connection was our connection, clean yport_conn */
    if (yport_conn == uip_conn)
      yport_conn = NULL;
  } else if (uip_newdata()) {
    if (uip_len <= YPORT_BUFFER_LEN && yport_rxstart(uip_appdata, uip_len) != 0) {
      /* Prevent the other side from sending more data */
      uip_stop();
    }
  }
  if (uip_poll()
      && uip_conn == yport_conn
      && uip_stopped(yport_conn)
      && yport_send_buffer.sent == yport_send_buffer.len)
    uip_restart();
  /* Send data */
  if ((uip_poll()
       || uip_acked()
       || uip_rexmit())
      && yport_conn == uip_conn
      && yport_recv_buffer.len > 0) {
    /* We have recieved data, lets propagade it */
    /* disable interrupts */
    uint8_t sreg = SREG; cli();
    /* Send the data */
    uip_send(yport_recv_buffer.data, yport_recv_buffer.len);
    /* so many data was send */
    yport_recv_buffer.sent = yport_recv_buffer.len;
    /* enable interrupts again */
    SREG = sreg;
  }
}
#endif
