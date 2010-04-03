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
#include "protocols/uip/uip_router.h"

#include <string.h>

static uip_udp_conn_t *ecmd_conn;
PGM_P send_pgm_data = NULL;
uint8_t resend_counter;
client_return_text_callback_t ucallback_p = NULL;

void
uecmd_sender_pgm_net_main(void) 
{
  if (uip_newdata()) {
    if(ucallback_p) {
      ucallback_p(uip_appdata, uip_len);
    }
    send_pgm_data = NULL;
  }
  if (send_pgm_data) {
    resend_counter --;
    if (!resend_counter) {
      if(ucallback_p) {
	ucallback_p(NULL, 0);
      }
      send_pgm_data = NULL;
      return;
    }
    uint8_t len = strlen_P(send_pgm_data);
    memcpy_P(uip_appdata, send_pgm_data, len);
    uip_slen = len;

    /* build a new connection on the stack */
    ecmd_conn->rport = HTONS(2701);

    uip_udp_conn = ecmd_conn;

    uip_process(UIP_UDP_SEND_CONN);
    router_output();
  }
}

void
uecmd_sender_pgm_send_command(uip_ipaddr_t *ipaddr, PGM_P pgm_data, client_return_text_callback_t callback) 
{
  if (send_pgm_data) {
    if(callback) {
      callback(NULL, 0);
    }
    return;
  }  

  if (!ecmd_conn) {
    ecmd_conn = uip_udp_new(ipaddr, 0, uecmd_sender_pgm_net_main);
    if (!ecmd_conn) {
      if(callback) {
	callback(NULL, 0);
      }
      return;
    }
  }
  uip_ipaddr_copy(ecmd_conn->ripaddr, ipaddr);
  send_pgm_data = pgm_data;
  ucallback_p = callback;
  resend_counter = 7;
}

