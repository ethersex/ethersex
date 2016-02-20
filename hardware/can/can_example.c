/*
 *
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

/*
 * Sample program for CAN-Bus and MCP2515
 *
 * To activate please insert "hardware/can/can_example.c" in the Makefile
 * under hardware/can
 */

#include "can_mcp2515.h"
#include "core/debug.h"

#if MCP2515_RXnBF_OUTPUT
static uint8_t status_rx0bf = 0;
static uint8_t status_rx1bf = 1;
#endif

const uint8_t can_filter[] PROGMEM = {
#if SUPPORT_EXTENDED_CANID

  // Group 0
  MCP2515_FILTER(0),            // Filter 0
  MCP2515_FILTER(0),            // Filter 1

  // Group 1
  MCP2515_FILTER_EXTENDED(0),   // Filter 2
  MCP2515_FILTER_EXTENDED(0),   // Filter 3
  MCP2515_FILTER_EXTENDED(0),   // Filter 4
  MCP2515_FILTER_EXTENDED(0),   // Filter 5

  MCP2515_FILTER(0),            // Mask 0 (for group 0)
  MCP2515_FILTER_EXTENDED(0),   // Mask 1 (for group 1)

#else
  // Group 0
  MCP2515_FILTER(0),            // Filter 0
  MCP2515_FILTER(0),            // Filter 1
  // Group 1
  MCP2515_FILTER(0),            // Filter 2
  MCP2515_FILTER(0),            // Filter 3
  MCP2515_FILTER(0),            // Filter 4
  MCP2515_FILTER(0),            // Filter 5

  MCP2515_FILTER(0),            // Mask 0 (for group 0)
  MCP2515_FILTER(0),            // Mask 1 (for group 1)
#endif
};

/*---------------------------------------------------------------------
 CAN init
 * --------------------------------------------------------------------*/
void
can_test_init()
{
  // Initialize MCP2515
  mcp2515_init(BITRATE_125_KBPS);
  // Filter
  mcp2515_static_filter(can_filter);
}

/*---------------------------------------------------------------------
 Reveive CAN Messages
 * --------------------------------------------------------------------*/
void
can_test_get_message()
{
  can_t message;

  if (mcp2515_get_message(&message))
  {
#ifdef DEBUG_MCP2515
    mcp2515_print_message(&message);
#endif
  }
}

/*---------------------------------------------------------------------
 Send CAN Messages
 * --------------------------------------------------------------------*/
void
can_test_send_message()
{
  can_t msg;
#if SUPPORT_EXTENDED_CANID
  msg.id = 0x1200AAAA;
  msg.flags.extended = 1;
#else
  msg.id = 0x510;
#endif
  msg.flags.rtr = 0;
  msg.length = 8;
  msg.data[0] = 0x1;
  msg.data[1] = 0x2;
  msg.data[2] = 0x3;
  msg.data[3] = 0x4;
  msg.data[4] = 0x5;
  msg.data[5] = 0x6;
  msg.data[6] = 0x7;
  msg.data[7] = 0x8;
  mcp2515_send_message(&msg);
}

/*---------------------------------------------------------------------
 Show statistic
 * --------------------------------------------------------------------*/
void
can_test_statistic(void)
{
  printf_P(PSTR("CAN  RX: %lu\n"), mcp2515_get_counter_messages_received());
  printf_P(PSTR("CAN  TX: %lu\n"), mcp2515_get_counter_messages_send());
}

/*---------------------------------------------------------------------
 Show Errors
 * --------------------------------------------------------------------*/
void
can_test_errors(void)
{
  can_error_register_t errors;

  errors = mcp2515_read_error_register();

  printf_P(PSTR("Empfangs-Register: %u\n"), errors.rx);
  printf_P(PSTR("Sende-Register: %u\n"), errors.tx);
  printf_P(PSTR("Error-Flags: %02X\n"), mcp2515_read_error_flags());
  printf_P(PSTR("--------------------------------------------\n"));
}

/*---------------------------------------------------------------------
 Send TX fifo
 * --------------------------------------------------------------------*/
void
can_test_send_tx_fifo(void)
{
  mcp2515_send_tx_fifo();
}

#if MCP2515_RXnBF_OUTPUT
/*---------------------------------------------------------------------
 RXnBF
 * --------------------------------------------------------------------*/
void
can_test_rxnbf(void)
{
  // RX0BF
  if (status_rx0bf)
  {
    mcp2515_rx0bf(0);
    status_rx0bf = 0;
  }
  else
  {
    mcp2515_rx0bf(1);
    status_rx0bf = 1;
  }

  // RX1BF
  if (status_rx1bf)
  {
    mcp2515_rx1bf(0);
    status_rx1bf = 0;
  }
  else
  {
    mcp2515_rx1bf(1);
    status_rx1bf = 1;
  }
}
#endif // MCP2515_RXnBF_OUTPUT

// timer(50, can_test_statistic())
// timer(15, can_test_rxnbf())
// timer(100, can_test_errors())

/*
 -- Ethersex META --
 header(hardware/can/can_example.h)

 init(can_test_init)

 mainloop(can_test_get_message)
 mainloop(can_test_send_tx_fifo)

 timer(20, can_test_send_message())
 */
