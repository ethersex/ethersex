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
#include <util/atomic.h>
#include <string.h>
#include "yport_net.h"
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "yport.h"

#include "config.h"

uip_conn_t *yport_conn = NULL;
#if YPORT_FLUSH > 0
static uint8_t yport_lastservice;
#endif

void
yport_net_init(void)
{
  uip_listen(HTONS(YPORT_PORT), yport_net_main);
}

void
yport_net_main(void)
{
#if YPORT_FLUSH > 0
  yport_lastservice++;
#endif

  if (uip_connected())
  {
    if (yport_conn == NULL)
    {
      yport_conn = uip_conn;
      uip_conn->wnd = YPORT_BUFFER_LEN - 1;
      /* delete the receive buffer */
      yport_recv_buffer.len = 0;
    }
    else
      /* if we already have a connection, send an error */
      uip_send("ERROR: connection blocked\n", 27);
  }
  else if (uip_acked())
  {
    /* if the peer is not our connection, close it */
    if (yport_conn != uip_conn)
      uip_close();
    else
    {
      /* data we have sent was acked */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        yport_recv_buffer.len -= yport_recv_buffer.sent;
        memmove(yport_recv_buffer.data,
                yport_recv_buffer.data + yport_recv_buffer.sent,
                yport_recv_buffer.len);
      }
    }
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {
    /* if the closed connection was our connection, clean yport_conn */
    if (yport_conn == uip_conn)
      yport_conn = NULL;
  }

  if (uip_newdata() && yport_conn == uip_conn)
  {
    if (uip_len <= YPORT_BUFFER_LEN &&
        yport_rxstart(uip_appdata, uip_len) != 0)
    {
      /* prevent the other side from sending more data via tcp */
      uip_stop();
    }
  }

  /* retransmit last packet */
  if (uip_rexmit() && yport_conn == uip_conn)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uip_send(yport_recv_buffer.data, yport_recv_buffer.sent);
    }
#ifdef DEBUG_YPORT
    yport_eth_retransmit++;
#endif
  }
  else
  {

    /* restart connection */
    if (uip_poll()
        && yport_conn == uip_conn
        && uip_stopped(yport_conn)
        && yport_send_buffer.sent == yport_send_buffer.len)
      uip_restart();

    /* send data */
    if ((uip_poll() || uip_acked()) && yport_conn == uip_conn
        /* receive buffer reached water mark */
#if YPORT_FLUSH > 0
        && (yport_recv_buffer.len > (YPORT_BUFFER_LEN / 4)
            /* last transmission is at least one second ago */
            || yport_lastservice >= YPORT_FLUSH
            /* we received a linefeed character, send immediately */
            || yport_lf)
#endif
      )
    {
      /* we have enough uart data, send it via tcp */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        uip_send(yport_recv_buffer.data, yport_recv_buffer.len);
        yport_recv_buffer.sent = yport_recv_buffer.len;
#if YPORT_FLUSH > 0
        yport_lastservice = 0;
        yport_lf = 0;
#endif
      }
    }
  }
}


void
yport_net_periodic(void)
{
  if (yport_conn)
  {
    uip_stack_set_active(yport_conn->stack);
    uip_conn = yport_conn;
    uip_process(UIP_TIMER);
    if (uip_len > 0)
      router_output();
  }
}

/*
  -- Ethersex META --
  header(protocols/yport/yport_net.h)
  net_init(yport_net_init)
  timer(1, yport_net_periodic())
*/
