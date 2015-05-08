/*
 *
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>

#include "core/debug.h"
#include "bsbport.h"
#include "bsbport_helper.h"
#include "bsbport_rx.h"

#ifdef DEBUG_BSBPORT_RX
#define BSBPORT_DEBUG(s, ...) debug_printf("BSB " s "\n", ## __VA_ARGS__);
#else
#define BSBPORT_DEBUG(a...) do {} while(0)
#endif

struct bsbport_buffer_msg bsbport_msg_buffer;

void
bsbport_rx_init(void)
{
  memset(&bsbport_msg_buffer, 0, sizeof(bsbport_msg_buffer));
}

void
bsbport_rx_periodic(void)
{
  uint8_t i;
  uint8_t buffer[BSBPORT_MESSAGE_MAX_LEN];

  while (bsbport_recv_buffer.len > bsbport_recv_buffer.read)
//      && bsbport_recv_buffer.len > 11)        // Minimal Message Size reached 
  {
    BSBPORT_DEBUG("Start Read: %u bytes avail", bsbport_recv_buffer.len);

    // Read serial data...
    if (bsbport_recv_buffer.data[bsbport_recv_buffer.read++] == SOT_BYTE) /* ... until SOT detected (= 0xDC)  */
    {
      i = 0;
      // Restore otherwise dropped SOF indicator
      buffer[i++] = SOT_BYTE;

      // read the rest of the message
      while (bsbport_recv_buffer.len > bsbport_recv_buffer.read)
      {
        buffer[i++] = bsbport_recv_buffer.data[bsbport_recv_buffer.read++];

        // Break if we are at max message lenght   
        // or if message seems to be completely received (i==msg.length)
        if (i >= BSBPORT_MESSAGE_MAX_LEN || ((i > LEN) && (i >= buffer[LEN])))
        {
          break;
        }
      }

      // Check if we have read the message completly.
      if (i > LEN && i == buffer[LEN])
      {
        // Seems to have received all data
        if (bsbport_crc(buffer, i) == 0)
        {
          BSBPORT_DEBUG
            ("Valid: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d",
             buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
             buffer[6], buffer[7], buffer[8], buffer[9], buffer[10],
             buffer[11], buffer[12], buffer[13], buffer[14], buffer[15],
             buffer[16], buffer[17], buffer[18], buffer[19], i);

          bsbport_rx_ok++;
          /*      Store Messages which contain information                                           */
          /*      Store Messages which are addressed to us                                           */
          /*      Valid Message received  -> If type = Answer or Info Get Value                   */
          /*      Valid Message received  -> If dest = OwnAddress or Info Get Value               */
          if (buffer[TYPE] == ANSWER
              || buffer[TYPE] == INFO
              || (buffer[DEST] & 0x0F) == BSBPORT_OWNADDRESS)
          {
            bsbport_store_msg(buffer, i);
          }

        }
        else                    /*  CRC Error */
        {
          BSBPORT_DEBUG
            ("CRC Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d",
             buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
             buffer[6], buffer[7], buffer[8], buffer[9], buffer[10],
             buffer[11], buffer[12], buffer[13], buffer[14], buffer[15],
             buffer[16], buffer[17], buffer[18], buffer[19], i);
          bsbport_rx_crcerror++;
        }
      }
      // We are still missing some bytes but we still have space in databuffer -> Reset Readbytes and try again next time
      else if (i <= LEN || (i < buffer[LEN] && i < BSBPORT_MESSAGE_MAX_LEN))
      {
        bsbport_recv_buffer.read = 0;
        return;
      }
      // Length error
      else if (i > LEN && i < buffer[LEN])
      {
        BSBPORT_DEBUG
          ("LUV Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d",
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
           buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11],
           buffer[12], buffer[13], buffer[14], buffer[15], buffer[16],
           buffer[17], buffer[18], buffer[19], i);
        bsbport_rx_lenghtunder++;
      }
      else if (i > LEN && i > buffer[LEN])
      {
        BSBPORT_DEBUG
          ("LOV Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d",
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
           buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11],
           buffer[12], buffer[13], buffer[14], buffer[15], buffer[16],
           buffer[17], buffer[18], buffer[19], i);
        bsbport_rx_lenghtover++;
      }
    }
    else
    {
      bsbport_rx_dropped++;
    }

    BSBPORT_DEBUG("Delete Bytes: %u ", bsbport_recv_buffer.read);

    /* data we have read can be cleared from buffer */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      bsbport_recv_buffer.len -= bsbport_recv_buffer.read;
      memmove(bsbport_recv_buffer.data,
              bsbport_recv_buffer.data + bsbport_recv_buffer.read,
              bsbport_recv_buffer.len);
      bsbport_recv_buffer.read = 0;
    }
  }
}

void
bsbport_calc_value(struct bsbport_msg *msg)
{
  BSBPORT_DEBUG("DATA: %02x %02x %02x Len:%d TYPE: %u", msg->data[0],
                 msg->data[1], msg->data[2], msg->data_length, msg->type);
// Nach Nachrichten Typ entscheiden
  if (msg->type == INFO)
  {
    msg->value = bsbport_ConvertToInt16(&msg->data[0]);
  }
  else if (msg->type == SET)
  {
    msg->value = bsbport_ConvertToInt16(&msg->data[0]);
  }
  else if (msg->type == ACK)
  {
    //Ack has no value
  }
  else if (msg->type == QUERY)
  {
    //Query has no value
  }
  else if (msg->type == ANSWER 
           && msg->data_length == 12
           && msg->p.data.p1 == 0x05
           && msg->p.data.p2 == 0x3D
           && msg->p.data.p3 == 0x00
           && msg->p.data.p4 == 0x9A)   // Msg with errorcode in byte 2 received
  {
    msg->value = (uint8_t) msg->data[1];
  }
  else if (msg->type == ANSWER && msg->data_length == 3)        // Msg with 3 Databytes received
  {
    msg->value = bsbport_ConvertToInt16(&msg->data[1]);
  }
  else if (msg->type == ANSWER && msg->data_length == 2)        // Msg with 2 Databytes received
  {
    msg->value = bsbport_ConvertToInt16(&msg->data[0]);
  }
  else if (msg->type == ANSWER) // Msg with unknow Databytes received
  {
    msg->value = bsbport_ConvertToInt16(&msg->data[1]);
  }
  else
  {
    //Unknown Type
    BSBPORT_DEBUG("Unknown Messagetype received: %02x", msg->type);
  }
  BSBPORT_DEBUG("Parsed as RAW %d", msg->value);

}

void
bsbport_store_msg(const uint8_t * const msg, const uint8_t len)
{
  uint8_t saved = 0;
  BSBPORT_DEBUG("Store MSG at POS: %d", bsbport_msg_buffer.act);
  for (uint8_t i = 0; i < BSBPORT_MESSAGE_BUFFER_LEN; i++)
  {
    if ((0x7F & bsbport_msg_buffer.msg[i].src) == (0x7F & msg[SRC])
        && bsbport_msg_buffer.msg[i].p.data.p1 == msg[P1]
        && bsbport_msg_buffer.msg[i].p.data.p2 == msg[P2]
        && bsbport_msg_buffer.msg[i].p.data.p3 == msg[P3]
        && bsbport_msg_buffer.msg[i].p.data.p4 == msg[P4])
    {
      // Mark message valid 
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        memcpy(bsbport_msg_buffer.msg[i].data, msg + DATA,
               msg[LEN] - DATA - 2);
        bsbport_msg_buffer.msg[i].data_length = msg[LEN] - DATA - 2;
        bsbport_calc_value(&bsbport_msg_buffer.msg[i]);
#ifdef BSBPORT_MQTT_SUPPORT
        bsbport_msg_buffer.msg[i].mqtt_new = 1;
#endif
        saved = 1;
      }
    }
  }
  if (!saved)
  {                             // Mark message valid 
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      memcpy(bsbport_msg_buffer.msg[bsbport_msg_buffer.act].data, msg + DATA,
             msg[LEN] - DATA - 2);
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].data_length =
        msg[LEN] - DATA - 2;
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].src = msg[SRC];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].dest = msg[DEST];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].type = msg[TYPE];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].p.data.p1 = msg[P1];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].p.data.p2 = msg[P2];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].p.data.p3 = msg[P3];
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].p.data.p4 = msg[P4];
#ifdef BSBPORT_MQTT_SUPPORT
      bsbport_msg_buffer.msg[bsbport_msg_buffer.act].mqtt_new = 1;
#endif
      bsbport_calc_value(&bsbport_msg_buffer.msg[bsbport_msg_buffer.act++]);
    }
  }
  if (bsbport_msg_buffer.act >= BSBPORT_MESSAGE_BUFFER_LEN)
    bsbport_msg_buffer.act = 0;
}

/*
  -- Ethersex META --
  header(protocols/bsbport/bsbport_rx.h)
  init(bsbport_rx_init)
  timer(5, bsbport_rx_periodic())
*/
