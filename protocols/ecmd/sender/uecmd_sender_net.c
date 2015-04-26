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
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "services/clock/clock.h"
#include "ecmd_sender_net.h"

typedef int (*fptr_t) (char *, size_t, const char *, va_list);


static void
uecmd_sender_net_main(void)
{
  struct ecmd_sender_connection_state_t *state =
    &uip_udp_conn->appstate.uecmd_sender;

  if (uip_newdata() && uip_len > 0)
  {
    if (state->callback != NULL)
    {
      state->callback(uip_appdata, uip_len);
      state->callback = NULL;
    }
    uip_udp_remove(uip_udp_conn);
    return;
  }

  if (--state->retry == 0)
  {
    if (state->callback)
    {
      state->callback(NULL, 0);
      state->callback = NULL;
    }
    uip_udp_remove(uip_udp_conn);
    return;
  }

  strncpy((char *) uip_appdata, state->buf, UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN);
  ((char *) uip_appdata)[UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN - 1] = 0;
  uip_udp_send(strlen((char *) uip_appdata));
}

static uip_udp_conn_t *
uecmd_sender_send(fptr_t f, uip_ipaddr_t * ipaddr,
                  client_return_text_callback_t callback,
                  const char *message, va_list ap)
{
  uip_udp_conn_t *conn = uip_udp_new(ipaddr, HTONS(ECMD_TCP_PORT),
                                     uecmd_sender_net_main);
  if (conn)
  {
    struct ecmd_sender_connection_state_t *state =
      &conn->appstate.uecmd_sender;
    f(state->buf, sizeof(state->buf), message, ap);
    state->buf[sizeof(state->buf) - 1] = 0;
    state->callback = callback;
    state->retry = 7;
  }
  else
  {
    callback(NULL, 0);
  }
  return conn;
}

uip_udp_conn_t *
uecmd_sender_send_command_P(uip_ipaddr_t * ipaddr,
                            client_return_text_callback_t callback,
                            const char *message, ...)
{
  va_list va;
  va_start(va, message);
  uip_udp_conn_t *conn = uecmd_sender_send(&vsnprintf_P, ipaddr,
                                           callback, message, va);
  va_end(va);
  return conn;
}

uip_udp_conn_t *
uecmd_sender_send_command(uip_ipaddr_t * ipaddr,
                          client_return_text_callback_t callback,
                          const char *message, ...)
{
  va_list va;
  va_start(va, message);
  uip_udp_conn_t *conn = uecmd_sender_send(&vsnprintf, ipaddr,
                                           callback, message, va);
  va_end(va);
  return conn;
}

/*
  -- Ethersex META --
  state_header(protocols/ecmd/sender/ecmd_sender_state.h)
  state_udp(struct ecmd_sender_connection_state_t uecmd_sender)
*/
