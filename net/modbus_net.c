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
  uint8_t i = 0;

  if(uip_connected()) {
    /* New connection */
    memset(&uip_conn->appstate, 0, sizeof(uip_conn->appstate));
  } else if (uip_acked()) {
    uip_conn->appstate.modbus.new_data = 0;
  } else if (uip_rexmit()) {
    if (uip_conn->appstate.modbus.new_data) 
      goto send_new_data;
  } else if (uip_closed() || uip_aborted() || uip_timedout()) {

  } else if (uip_newdata()) {
    /* Have we space for a new packet? */
    if (STATE(uip_conn).must_send || STATE(uip_conn).new_data) {
      /* we don't have enough space, sent error */
      answer[8] = 0x06; // Server busy
      goto error_response;
    }
    if (answer[5] > MODBUS_BUFFER_LEN) {
      answer[8] = 0x04; // Server failure
      goto error_response;
    }
    memcpy(STATE(uip_conn).data, answer + 6, answer[5]);
    STATE(uip_conn).len = answer[5];
    STATE(uip_conn).transaction_id = (answer[0] | (answer[1] << 8));
    STATE(uip_conn).must_send = 1;
  } 

  /* See if we can send new data */
  if (modbus_conn == NULL) 
    /* Search for a connetion with data, that had to be send */
    for (i = 0; i < UIP_CONNS; i ++) 
      if (uip_conns[i].callback == modbus_net_main
          && uip_conns[i].tcpstateflags != UIP_CLOSED) {
          if (uip_conns[i].appstate.modbus.must_send) {
            modbus_conn = &uip_conns[i];
            /* Start the transmission */
            modbus_rxstart((uint8_t *)STATE(modbus_conn).data, STATE(modbus_conn).len);
            break;
          } else if (uip_conns[i].appstate.modbus.new_data) {
            uip_conn = &uip_conns[i];
send_new_data:
            if (STATE(uip_conn).len == 0) {
              if (STATE(uip_conn).data[0] >= 0xf0) {
                uip_conn->appstate.modbus.new_data = 0;
                continue;
              }
              // Send an error message
             answer[8] = 0x0B; // gateway problem
             goto error_response;
            }
            uint16_t crc = modbus_crc_calc(STATE(uip_conn).data, STATE(uip_conn).len - 2);
            uint16_t crc_recv = 
                    ((STATE(uip_conn).data[STATE(uip_conn).len - 1])  << 8) 
                    | (STATE(uip_conn).data[STATE(uip_conn).len - 2]);
            if (crc != crc_recv) {
              // Send an error message
             answer[8] = 0x0B; // gateway problem
             goto error_response;
            }
            memcpy(answer + 6, STATE(uip_conn).data,
                   STATE(uip_conn).len - 2);
            memset(answer, 0, 6);

            answer[0] = STATE(uip_conn).transaction_id;
            answer[1] = STATE(uip_conn).transaction_id >> 8;
            answer[5] = STATE(uip_conn).len - 2;

            uip_udp_send(STATE(uip_conn).len -2 +6);
          }

      }

  return;

error_response:
  /* The length of the answer */
  answer[5] = 3;
  answer[4] = 0;
  /* Function code manipulation */
  answer[7] |= 0x80;
  uip_send(answer, 9);
}
#endif
