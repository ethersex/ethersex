/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include "ecmd_sender_net.h"
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../debug.h"

#include <string.h>

#include "../config.h"
#ifdef ECMD_SENDER_SUPPORT

/* module local prototypes */
struct uip_conn*
ecmd_sender_send_command(uip_ipaddr_t *ipaddr, const char *pgm_data)
{
  struct uip_conn *conn = uip_connect(ipaddr, HTONS(2701));
  if (conn) {
    conn->appstate.ecmd_sender.to_be_sent = pgm_data;
    conn->appstate.ecmd_sender.offset = 0;
  }
  return conn;
}

void ecmd_sender_net_main(void)
{
  char buffer[100];
  uint8_t len;
  struct ecmd_sender_connection_state_t *state = &uip_conn->appstate.ecmd_sender;


  if(uip_acked()) {
    if (strlen_P(state->to_be_sent + state->offset) > sizeof(buffer))
      state->offset += sizeof(buffer);
    else
      uip_close();
  }

  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked() ||
     uip_connected() ||
     uip_poll()) {
    strncpy_P(buffer, state->to_be_sent + state->offset, 
              sizeof(buffer));
    if (buffer[sizeof(buffer) - 1] == 0)
      len = strlen(buffer);
    else
      len = sizeof(buffer);
    uip_send(buffer, len);
  }
}


#endif /* ECMD_SENDER_SUPPORT */
