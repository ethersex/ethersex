/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2015 by Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "ecmd_sender_net.h"


typedef int (*fptr_t) (char *, size_t, const char *, va_list);


static void
ecmd_sender_net_main(void)
{
  struct ecmd_sender_connection_state_t *state =
    &uip_conn->appstate.ecmd_sender;

  /* Response */
  if (uip_newdata() && uip_len > 0)
  {
    if (state->callback != NULL)
    {
      state->callback(uip_appdata, uip_len);
      state->callback = NULL;   /* we must set this to NULL, because 
                                 * otherwise it would be called on close */
    }
    uip_close();
  }

  if (uip_closed() && state->callback != NULL)
  {
    state->callback(NULL, 0);
    state->callback = NULL;
  }

  if (uip_acked())
    state->sent = 1;

  if (uip_rexmit() || uip_acked() || uip_connected() || uip_poll())
  {
    /* Send one bufer of data */
    if (!state->sent)
    {
      strncpy((char *) uip_appdata, state->buf, uip_mss() - 1);
      ((char *) uip_appdata)[uip_mss() - 2] = 0;
      uip_send((char *) uip_appdata, strlen((char *) uip_appdata));
    }
  }
}

static uip_conn_t *
ecmd_sender_send(fptr_t f, uip_ipaddr_t * ipaddr,
                 client_return_text_callback_t callback,
                 const char *message, va_list ap)
{
  uip_conn_t *conn = uip_connect(ipaddr, HTONS(ECMD_TCP_PORT),
                                 ecmd_sender_net_main);
  if (conn)
  {
    struct ecmd_sender_connection_state_t *state =
      &conn->appstate.ecmd_sender;
    f(state->buf, sizeof(state->buf), message, ap);
    state->buf[sizeof(state->buf) - 1] = 0;
    state->callback = callback;
    state->sent = 0;
  }
  else
  {
    callback(NULL, 0);
  }
  return conn;
}

uip_conn_t *
ecmd_sender_send_command_P(uip_ipaddr_t * ipaddr,
                           client_return_text_callback_t callback,
                           const char *message, ...)
{
  va_list va;
  va_start(va, message);
  uip_conn_t *conn = ecmd_sender_send(&vsnprintf_P, ipaddr,
                                      callback, message, va);
  va_end(va);
  return conn;
}

uip_conn_t *
ecmd_sender_send_command(uip_ipaddr_t * ipaddr,
                         client_return_text_callback_t callback,
                         const char *message, ...)
{
  va_list va;
  va_start(va, message);
  uip_conn_t *conn = ecmd_sender_send(&vsnprintf, ipaddr,
                                      callback, message, va);
  va_end(va);
  return conn;
}

/*
  -- Ethersex META --
  state_header(protocols/ecmd/sender/ecmd_sender_state.h)
  state_tcp(struct ecmd_sender_connection_state_t ecmd_sender)
*/
