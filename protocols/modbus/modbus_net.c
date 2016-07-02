/*
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
 */

#include <string.h>
#include <avr/io.h>

#include "modbus_net.h"
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "protocols/modbus/modbus.h"

#include "config.h"

#define STATE(a) ((a)->appstate.modbus)


extern int16_t *modbus_recv_len_ptr;
static int16_t recv_len = -2;

void
modbus_net_init(void)
{
  uip_listen(HTONS(MODBUS_PORT), modbus_net_main);
}

void
modbus_net_main(void)
{
  uint8_t *answer = uip_appdata;
  uint8_t i = 0;

  if (uip_connected())
  {
    /* New connection */
    memset(&uip_conn->appstate, 0, sizeof(uip_conn->appstate));
  }
  else if (uip_acked())
  {
    uip_conn->appstate.modbus.state = MODBUS_IDLE;
  }
  else if (uip_rexmit())
  {
    if (uip_conn->appstate.modbus.state == MODBUS_MUST_ANSWER)
      goto send_new_data;
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {

  }
  if (uip_newdata())
  {
    /* Have we space for a new packet? */
    if (STATE(uip_conn).state != MODBUS_IDLE)
    {
      /* we don't have enough space, sent error */
      answer[8] = 0x06;         // Server busy
      goto error_response;
    }
    if (answer[5] > MODBUS_BUFFER_LEN)
    {
      answer[8] = 0x04;         // Server failure
      goto error_response;
    }
    memcpy(STATE(uip_conn).data, answer + 6, answer[5]);
    STATE(uip_conn).len = answer[5];
    STATE(uip_conn).transaction_id = (answer[0] | (answer[1] << 8));
    STATE(uip_conn).state = MODBUS_MUST_SEND;
  }

  /* See if we can send new data */
  if (recv_len != 0 || !modbus_recv_len_ptr)
    /* Search for a connetion with data, that had to be send */
    for (i = 0; i < UIP_CONNS; i++)
      if (uip_conns[i].callback == modbus_net_main &&
          uip_conns[i].tcpstateflags != UIP_CLOSED)
      {
        if (uip_conns[i].appstate.modbus.state == MODBUS_MUST_SEND)
        {
          /* Start the transmission */
          recv_len = 0;
          modbus_rxstart((uint8_t *) STATE(&uip_conns[i]).data,
                         STATE(&uip_conns[i]).len, &recv_len);
          STATE(&uip_conns[i]).state = MODBUS_WAIT_ANSWER;
          break;
        }
        else if (STATE(&uip_conns[i]).state == MODBUS_WAIT_ANSWER
                 && recv_len != 0)
        {
          uip_conn = &uip_conns[i];
          STATE(uip_conn).state = MODBUS_MUST_ANSWER;
        send_new_data:
          if (recv_len == -1)
          {
            // Send an error message
            answer[8] = 0x05;   // gateway problem
            goto error_response;
          }
          uint16_t crc = modbus_crc_calc(STATE(uip_conn).data, recv_len - 2);
          uint16_t crc_recv =
            ((STATE(uip_conn).data[recv_len - 1]) << 8)
            | (STATE(uip_conn).data[recv_len - 2]);
          if (crc != crc_recv)
          {
            // Send an error message
            answer[8] = 0x0B;   // gateway problem
            goto error_response;
          }
          memcpy(answer + 6, STATE(uip_conn).data, recv_len - 2);
          memset(answer, 0, 6);

          answer[0] = STATE(uip_conn).transaction_id;
          answer[1] = STATE(uip_conn).transaction_id >> 8;
          answer[5] = recv_len - 2;

          uip_udp_send(recv_len - 2 + 6);
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

/*
  -- Ethersex META --
  header(protocols/modbus/modbus_net.h)
  net_init(modbus_net_init)

  state_header(protocols/modbus/modbus_state.h)
  state_tcp(struct modbus_connection_state_t modbus)
*/
