/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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
#include "core/heartbeat.h"
#include "ems.h"

struct ems_uart_input_buffer ems_input_buffer;
struct ems_buffer ems_send_buffer;
struct ems_buffer ems_recv_buffer;
#ifdef EMS_DEBUG_STATS
struct ems_stats ems_stats_buffer;
#endif
uint8_t led_timeout[EMS_NUM_LEDS];
uint8_t ems_poll_address;
static uint8_t last_destination = 0;
static uint8_t dest_counter = 0;

void
ems_init(void)
{
  ems_uart_init();

#ifdef STATUSLED_EMS_TX_SUPPORT
  PIN_CLEAR(STATUSLED_EMS_TX);
#endif
#ifdef STATUSLED_EMS_RX_FAIL_SUPPORT
  PIN_CLEAR(STATUSLED_EMS_RX_FAIL);
#endif
#ifdef STATUSLED_EMS_RX_OK_SUPPORT
  PIN_CLEAR(STATUSLED_EMS_RX_OK);
#endif
}

void
ems_periodic_timeout(void)
{
  uint8_t i;
  for (i = 0; i < EMS_NUM_LEDS; i++) {
    if (led_timeout[i] > 0) {
      led_timeout[i]--;
      if (led_timeout[i] == 0) {
        ems_set_led(i, 0, 0);
      }
    }
  }
}

void
ems_set_led(uint8_t led, uint8_t enable, uint8_t timeout)
{
  switch (led) {
    case LED_TX:
#ifdef STATUSLED_EMS_TX_SUPPORT
      if (enable) PIN_SET(STATUSLED_EMS_TX);
      else PIN_CLEAR(STATUSLED_EMS_TX);
#endif
      ACTIVITY_LED_EMS_TX;
      break;
    case LED_RX_OK:
#ifdef STATUSLED_EMS_RX_OK_SUPPORT
      if (enable) PIN_SET(STATUSLED_EMS_RX_OK);
      else PIN_CLEAR(STATUSLED_EMS_RX_OK);
#endif
      ACTIVITY_LED_EMS_RX;
      break;
    case LED_RX_FAIL:
#ifdef STATUSLED_EMS_RX_FAIL_SUPPORT
      if (enable) PIN_SET(STATUSLED_EMS_RX_FAIL);
      else PIN_CLEAR(STATUSLED_EMS_RX_FAIL);
#endif
      break;
    default:
      return;
  }

  led_timeout[led] = enable ? timeout : 0;
}

uint8_t
ems_process_txdata(uint8_t *data, uint16_t len)
{
  uint16_t diff = ems_send_buffer.len - ems_send_buffer.sent;
  if (diff == 0 && len <= (EMS_BUFFER_LEN - 1)) {
    /* Copy the data to the send buffer */
    ems_send_buffer.addr_byte = OUR_EMS_ADDRESS;
    memcpy(ems_send_buffer.data, data, len);
    ems_send_buffer.data[len] =
        ems_calc_checksum(&ems_send_buffer.addr_byte, len + 1);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      ems_send_buffer.len = len + 1 /* data + checksum */;
      ems_send_buffer.sent = 0;
    }
  } else {
    return 0;
  }

  return 1;
}

void
ems_uart_process_input_byte(uint8_t data, uint8_t status)
{
  static uint8_t packet_bytes = 0;
  static uint8_t last_data;
  uint8_t index = ems_input_buffer.count;

  if (status & FRAMEEND) {
    /* end-of-frame */
    ems_input_buffer.data[index] = 0;
    ems_input_buffer.eop[EOP_BYTE(index)] |= EOP_BIT(index);
    ems_input_buffer.count++;
    ems_poll_address = (packet_bytes == 1) ? last_data : 0;
    if (packet_bytes > 1 && last_destination == OUR_EMS_ADDRESS) {
      ems_uart_got_response();
    }
    EMSIODEBUG("Got packet with %d bytes\n", packet_bytes);
    packet_bytes = 0;
    dest_counter = 0;
    last_destination = 0;
  } else if (status & ERROR) {
    /* error -> drop */
  } else {
    ems_input_buffer.data[index] = data;
    ems_input_buffer.count++;
    last_data = data;
    packet_bytes++;
    dest_counter++;
    if (dest_counter == 2) {
      last_destination = data;
    }
  }

  if (ems_input_buffer.count >= EMS_UART_INPUT_BUFSIZE) {
    EMSERRORDEBUG("Input buffer overflow\n");
    UPDATE_STATS(buffer_overflow, 1);
    ems_input_buffer.count = 0;
  }
}

/*
 * Mark EOP byte as coming from a certain source
 */
void
ems_add_source_to_eop(uint8_t source)
{
  uint8_t index = ems_input_buffer.count - 1;

  if (ems_input_buffer.count == 0) {
    return;
  }
  if (ems_input_buffer.data[index] == 0) {
    if (ems_input_buffer.eop[EOP_BYTE(index)] & EOP_BIT(index)) {
      ems_input_buffer.data[index] = source;
    }
  }
}

/*
  -- Ethersex META --
  dnl ems_init call must be done after network_init (according to earlier
  dnl comments.  Therefore we initialize via net_init and control the
  dnl order via the Makefile.

  header(protocols/ems/ems.h)
  net_init(ems_init)
  timer(5, ems_periodic_timeout())
*/
