/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "modbus_net.h"
#include "../uip/uip.h"
#include "../debug.h"
#include "../modbus/modbus.h"

#include "../config.h"

#ifdef MODBUS_SUPPORT
#define STATE(a) ((a)->appstate.modbus)

volatile uip_conn_t *modbus_conn = NULL;

void modbus_net_init(void)
{
  uip_listen(HTONS(MODBUS_PORT), modbus_net_main);
}

void modbus_net_main(void)
{
  uint8_t *answer = uip_appdata;
  uint8_t i;

  if(uip_connected()) {
    /* New connection */
    memset(&uip_conn->appstate, 0, sizeof(uip_conn->appstate));
  } else if (uip_acked()) {
  } else if (uip_closed() || uip_aborted() || uip_timedout()) {

  } else if (uip_newdata()) {
    /* Have we space for a new packet? */
    if (STATE(uip_conn).must_send) {
      /* we don't have enough space, sent error */
      answer[8] = 0x06;
      goto error_response;
    }
    memcpy(STATE(uip_conn).data, answer + 6, answer[4] + 1);
    STATE(uip_conn).len = answer[4] + 1;
    STATE(uip_conn).transaction_id = (answer[0] | (answer[1] << 8));
    STATE(uip_conn).must_send = 1;
  } 

  /* See if we can send new data */
  if (modbus_conn == NULL) 
    /* Search for a connetion with data, that had to be send */
    for (i = 0; i < UIP_CONNS; i ++) 
      if (uip_conns[i].callback == modbus_net_main
          && uip_conns[i].tcpstateflags != UIP_CLOSED
          && uip_conns[i].appstate.modbus.must_send) {
        modbus_conn = &uip_conns[i];
        /* Start the transmission */
        modbus_rxstart((uint8_t *)STATE(modbus_conn).data, STATE(modbus_conn).len);
        break;
      }

  return;

error_response:
  /* The length of the answer */
  answer[4] = 2;
  answer[5] = 0;
  /* Function code manipulation */
  answer[7] |= 0x80;


}
#endif
