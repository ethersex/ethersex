/*
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
 */

#include "config.h"
#include "ecmd_sender_net.h"
#include "protocols/uip/uip.h"
#include "core/debug.h"

#include <string.h>

/* module local prototypes */

uip_conn_t *
ecmd_sender_send_command(uip_ipaddr_t *ipaddr, const char *pgm_data, 
                         client_return_text_callback_t callback)
{
  uip_conn_t *conn = uip_connect(ipaddr, HTONS(2701), ecmd_sender_net_main);
  if (conn) {
    conn->appstate.ecmd_sender.to_be_sent = pgm_data;
    conn->appstate.ecmd_sender.callback = callback;
    conn->appstate.ecmd_sender.sent = 0;
  }
  return conn;
}

void ecmd_sender_net_main(void)
{
  struct ecmd_sender_connection_state_t *state = &uip_conn->appstate.ecmd_sender;

  if(uip_newdata() && uip_len > 0 ) { //&& !uip_connected()) {
    if (state->callback != NULL) {
      state->callback(uip_appdata, uip_len);
      state->callback = NULL; /* we must set this to NULL, because 
                                 otherwise it would be called on close */
    }
    uip_close();    
  }

  if (uip_closed() && state->callback != NULL) {
      state->callback(NULL, 0);
      state->callback = NULL;
  }

  if(uip_acked()) 
    state->sent = 1;

  if(uip_rexmit() ||
     uip_acked() ||
     uip_connected() ||
     uip_poll()) {
    /* Send one buffer of data */
    if (state->sent) return;
    ((char *) uip_appdata)[sizeof(uip_appdata) - 1] = 0;
    strncpy_P((char *)uip_appdata, state->to_be_sent, uip_mss() - 1);
    uip_udp_send(strlen((char *) uip_appdata));
  }
}

/*
  -- Ethersex META --
  state_header(protocols/ecmd/sender/ecmd_sender_state.h)
  state_tcp(struct ecmd_sender_connection_state_t ecmd_sender)
*/
