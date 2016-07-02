/*
 * Copyright (c) 2011 by Danny Baumann <dannybaumann@web.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3)
 * as published by the Free Software Foundation.
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

#include <string.h>
#include <util/atomic.h>
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "ems.h"

#include "config.h"

uip_conn_t *ems_conn = NULL;

void
ems_net_init(void)
{
  uip_listen(HTONS(EMS_PORT), ems_net_main);
}

uint8_t
ems_net_connected(void)
{
  return ems_conn != NULL;
}

void
ems_net_main(void)
{
  if (uip_connected())
  {
    if (ems_conn == NULL)
    {
      ems_conn = uip_conn;
      uip_conn->wnd = EMS_BUFFER_LEN - 1;
    }
    else
    {
      /* if we have already an connection, send an error */
      uip_send("ERROR: Connection blocked\n", 27);
    }
  }
  else if (uip_acked())
  {
    /* If the peer is not our connection, close it */
    if (ems_conn != uip_conn)
    {
      uip_close();
    }
    else
    {
      /* Some data we have sent was acked, jipphie */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        ems_recv_buffer.len -= ems_recv_buffer.sent;
        memmove(ems_recv_buffer.data,
                ems_recv_buffer.data + ems_recv_buffer.sent,
                ems_recv_buffer.len);
      }
    }
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {
    /* if the closed connection was our connection, clean ems_conn */
    if (ems_conn == uip_conn)
    {
      ems_conn = NULL;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        ems_recv_buffer.len = 0;
      }
    }
  }
  if (uip_newdata() && ems_conn == uip_conn)
  {
    if (uip_len <= EMS_BUFFER_LEN)
    {
      if (ems_process_txdata(uip_appdata, uip_len) != 0)
      {
        /* Prevent the other side from sending more data */
        uip_stop();
      }
    }
  }
  if (uip_poll()
      && uip_conn == ems_conn
      && uip_stopped(ems_conn) && ems_send_buffer.sent == ems_send_buffer.len)
  {
    uip_restart();
  }
  /* Send data */
  if ((uip_poll() || uip_acked() || uip_rexmit())
      && ems_conn == uip_conn && ems_recv_buffer.len > 0)
  {
    /* We have received data, lets propagate it */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uip_send(ems_recv_buffer.data, ems_recv_buffer.len);
      ems_recv_buffer.sent = ems_recv_buffer.len;
    }
  }
}

/*
  -- Ethersex META --
  net_init(ems_net_init)
*/
