/*
 * Copyright (c) 2013 by Frank Sautter <ethersix@sautter.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>
#include "pport_net.h"
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "pport.h"

#include "config.h"

uip_conn_t *pport_conn = NULL;
#if PPORT_FLUSH > 0
static uint8_t pport_lastservice;
#endif

void
pport_net_init(void)
{
  uip_listen(HTONS(PPORT_PORT), pport_net_main);
}

void
pport_net_main(void)
{
#if PPORT_FLUSH > 0
  pport_lastservice++;
#endif

  if (uip_connected())
  {
    if (pport_conn == NULL)
    {
      pport_conn = uip_conn;
      uip_conn->wnd = PPORT_BUFFER_LEN - 1;
    }
    else
      /* if we already have a connection, send an error */
      uip_send("ERROR: connection blocked\n", 27);
  }
  else if (uip_acked())
  {
    /* if the peer is not our connection, close it */
    if (pport_conn != uip_conn)
      uip_close();
    else
    {
      /* data we have sent was acked */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        pport_recv_buffer.len -= pport_recv_buffer.sent;
        memmove(pport_recv_buffer.data,
                pport_recv_buffer.data + pport_recv_buffer.sent,
                pport_recv_buffer.len);

        if (pport_delayed_ack)
        {
          PIN_CLEAR(PPORT_ACK);             // send /ACK signal
#ifndef PPORT_HAVE_74112
          PIN_CLEAR(PPORT_BUSY);            // set BUSY inactive
#endif
          PIN_SET(PPORT_ACK);
          pport_delayed_ack = 0;
        }
      }
    }
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {
    /* if the closed connection was our connection, clean pport_conn */
    if (pport_conn == uip_conn)
      pport_conn = NULL;
  }

  /* retransmit last packet */
  if (uip_rexmit() && pport_conn == uip_conn)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uip_send(pport_recv_buffer.data, pport_recv_buffer.sent);
    }
#ifdef DEBUG_PPORT
    pport_eth_retransmit++;
#endif
  }
  else
  {

    /* restart connection */
    if (uip_poll()
        && pport_conn == uip_conn
        && uip_stopped(pport_conn))
      uip_restart();

    /* send data */
    if ((uip_poll() || uip_acked()) && pport_conn == uip_conn
        /* receive buffer reached water mark */
#if PPORT_FLUSH > 0
        && (pport_recv_buffer.len > (PPORT_BUFFER_LEN / 4)
            /* last transmission is at least one second ago */
            || pport_lastservice >= PPORT_FLUSH
            /* we received a linefeed character, send immediately */
            || pport_lf)
#endif
      )
    {
      /* we have enough data, send it via tcp */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        uip_send(pport_recv_buffer.data, pport_recv_buffer.len);
        pport_recv_buffer.sent = pport_recv_buffer.len;
#if PPORT_FLUSH > 0
        pport_lastservice = 0;
        pport_lf = 0;
#endif
      }
    }
  }
}


void
pport_net_periodic(void)
{
  if (pport_conn)
  {
    uip_stack_set_active(pport_conn->stack);
    uip_conn = pport_conn;
    uip_process(UIP_TIMER);
    if (uip_len > 0)
      router_output();
  }
}

/*
  -- Ethersex META --
  header(protocols/pport/pport_net.h)
  net_init(pport_net_init)
  timer(1, pport_net_periodic())
*/
