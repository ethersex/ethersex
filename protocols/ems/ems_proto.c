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

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>
#include "config.h"
#include "ems.h"

static struct ems_uart_input_buffer buffer_shadow;
static uint8_t prepare_buffer[64];
static uint8_t prepare_fill = 0;

uint8_t
ems_calc_checksum(const uint8_t *buffer, uint8_t size)
{
  uint8_t crc = 0, d;

  for (uint8_t i = 0; i < size; i++) {
    d = 0;
    if (crc & 0x80) {
      crc ^= 0xc;
      d = 1;
    }
    crc <<= 1;
    crc &= 0xfe;
    crc |= d;
    crc = crc ^ buffer[i];
  }

  return crc;
}


static void
process_prepare_buffer(uint8_t source_byte)
{
  UPDATE_STATS(total_bytes, prepare_fill);

  if (prepare_fill == 1) {
    if (source_byte == 0) {
      UPDATE_STATS(onebyte_packets, 1);
      return;
    } else {
      /* this was ack or nack, construct fake packet */
      prepare_buffer[3] = prepare_buffer[0];
      prepare_buffer[0] = source_byte;
      prepare_buffer[1] = OUR_EMS_ADDRESS;
      prepare_buffer[2] = EMS_MSG_TYPE_RESP;
      prepare_fill = 4;
    }
  } else {
    /* strip CRC */
    prepare_fill--;

    uint8_t crc = ems_calc_checksum(prepare_buffer, prepare_fill);
    EMSPROTODEBUG("Packet CRC %02x calc %02x\n",
                  prepare_buffer[prepare_fill], crc);
    if (crc != prepare_buffer[prepare_fill]) {
      UPDATE_STATS(bad_packets, 1);
      ems_set_led(LED_RX_FAIL, 1, 2);
      return;
    }
  }

  uint16_t needed = prepare_fill + 4; /* frame start + len + csum */
  if (ems_recv_buffer.len + needed >= EMS_BUFFER_LEN) {
    EMSERRORDEBUG("Output buffer too small (has %d need %d)\n",
                  EMS_BUFFER_LEN - ems_recv_buffer.len, needed);
    UPDATE_STATS(dropped_packets, 1);
    UPDATE_STATS(dropped_bytes, prepare_fill);
    return;
  }

  UPDATE_STATS(good_bytes, prepare_fill);
  UPDATE_STATS(good_packets, 1);
  ems_set_led(LED_RX_OK, 1, 1);

  if (ems_net_connected()) {
    /* start-of-frame */
    ems_recv_buffer.data[ems_recv_buffer.len++] = 0xaa;
    ems_recv_buffer.data[ems_recv_buffer.len++] = 0x55;

    /* len */
    ems_recv_buffer.data[ems_recv_buffer.len++] = prepare_fill;

    /* data */
    uint8_t csum = 0;
    for (uint8_t i = 0; i < prepare_fill; i++) {
      uint8_t byte = prepare_buffer[i];
      ems_recv_buffer.data[ems_recv_buffer.len++] = byte;
      csum ^= byte;
    }

    /* checksum */
    ems_recv_buffer.data[ems_recv_buffer.len++] = csum;
    EMSPROTODEBUG("Send %d byte packet to client\n", prepare_fill);
  } else {
    EMSPROTODEBUG("No client connected, dropping %d bytes\n", prepare_fill);
  }
}

void
ems_process(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    buffer_shadow.count = ems_input_buffer.count;
    if (ems_input_buffer.count > 0) {
      memcpy(buffer_shadow.data, ems_input_buffer.data,
             ems_input_buffer.count);
      memcpy(buffer_shadow.eop, ems_input_buffer.eop,
             sizeof(buffer_shadow.eop));
      ems_input_buffer.count = 0;
      memset(ems_input_buffer.eop, 0, sizeof(ems_input_buffer.eop));
    }
  }

#ifdef EMS_DEBUG_STATS
  if (buffer_shadow.count > ems_stats_buffer.max_fill) {
    ems_stats_buffer.max_fill = buffer_shadow.count;
  }
#endif

  for (uint8_t i = 0; i < buffer_shadow.count; i++) {
    if (buffer_shadow.eop[EOP_BYTE(i)] & EOP_BIT(i)) {
      if (prepare_fill > 0) {
        EMSIODEBUG("Got %d bytes from input buf\n", prepare_fill);
        process_prepare_buffer(buffer_shadow.data[i]);
      }
      prepare_fill = 0;
    } else if (prepare_fill < sizeof(prepare_buffer)) {
      prepare_buffer[prepare_fill++] = buffer_shadow.data[i];
    } else {
      UPDATE_STATS(buffer_overflow, 1);
      prepare_fill = 0;
    }
  }
}

/*
  -- Ethersex META --
  header(protocols/ems/ems.h)
  mainloop(ems_process)
*/
