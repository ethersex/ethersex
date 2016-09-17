/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include "bsbport_net.h"
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "bsbport.h"

#include "config.h"

uip_conn_t *bsbport_conn = NULL;
#if BSBPORT_FLUSH > 0
static uint8_t bsbport_lastservice;
#endif

void
bsbport_net_init(void)
{
  uip_listen(HTONS(BSBPORT_PORT), bsbport_net_main);
}

void
bsbport_net_main(void)
{
#if BSBPORT_FLUSH > 0
  bsbport_lastservice++;
#endif

  if (uip_connected())
  {
    if (bsbport_conn == NULL)
    {
      bsbport_conn = uip_conn;
      uip_conn->wnd = BSBPORT_BUFFER_LEN - 1;
    }
    else
      /* if we already have a connection, send an error */
      uip_send("ERROR: connection blocked\n", 27);
  }
  else if (uip_acked())
  {
    /* if the peer is not our connection, close it */
    if (bsbport_conn != uip_conn)
      uip_close();
    else
    {
      /* data we have sent was acked */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        bsbport_recvnet_buffer.len -= bsbport_recvnet_buffer.sent;
        memmove(bsbport_recvnet_buffer.data,
                bsbport_recvnet_buffer.data + bsbport_recvnet_buffer.sent,
                bsbport_recvnet_buffer.len);
      }
    }
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {
    /* if the closed connection was our connection, clean bsbport_conn */
    if (bsbport_conn == uip_conn)
      bsbport_conn = NULL;
  }
  if (uip_newdata() && bsbport_conn == uip_conn)
  {
    if (uip_len <= BSBPORT_BUFFER_LEN &&
        bsbport_txstart(uip_appdata, uip_len) != 0)
    {
      /* prevent the other side from sending more data via tcp */
      uip_stop();
    }
  }

  /* retransmit last packet */
  if (uip_rexmit() && bsbport_conn == uip_conn)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uip_send(bsbport_recvnet_buffer.data, bsbport_recvnet_buffer.sent);
    }
    bsbport_eth_retransmit++;
  }
  else
  {

    /* restart connection */
    if (uip_poll()
        && bsbport_conn == uip_conn
        && uip_stopped(bsbport_conn)
        && bsbport_send_buffer.sent == bsbport_send_buffer.len)
      uip_restart();

    /* send data */
    if ((uip_poll() || uip_acked()) && bsbport_conn == uip_conn
        /* receive buffer reached water mark */
#if BSBPORT_FLUSH > 0
        && (bsbport_recvnet_buffer.len > (BSBPORT_BUFFER_LEN / 4)
            /* last transmission is at least one second ago */
            || bsbport_lastservice >= BSBPORT_FLUSH
            /* we received a linefeed character, send immediately */
            || bsbport_lf)
#endif
      )
    {
      /* we have enough uart data, send it via tcp */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        uip_send(bsbport_recvnet_buffer.data, bsbport_recvnet_buffer.len);
        bsbport_recvnet_buffer.sent = bsbport_recvnet_buffer.len;
#if BSBPORT_FLUSH > 0
        bsbport_lastservice = 0;
        bsbport_lf = 0;
#endif
      }
    }
  }
}


void
bsbport_net_periodic(void)
{
  if (bsbport_conn)
  {
    uip_stack_set_active(bsbport_conn->stack);
    uip_conn = bsbport_conn;
    uip_process(UIP_TIMER);
    if (uip_len > 0)
      router_output();
  }
}

/*
  -- Ethersex META --
  header(protocols/bsbport/bsbport_net.h)
  net_init(bsbport_net_init)
  timer(1, bsbport_net_periodic())
*/
