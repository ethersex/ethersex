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

#ifndef _CAN_MCP2515_H
#define _CAN_MCP2515_H

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef enum
{
  BITRATE_10_KBPS = 0,
  BITRATE_20_KBPS = 1,
  BITRATE_50_KBPS = 2,
  BITRATE_100_KBPS = 3,
  BITRATE_125_KBPS = 4,
  BITRATE_250_KBPS = 5,
  BITRATE_500_KBPS = 6,
  BITRATE_1_MBPS = 7,
} can_bitrate_t;

#define MCP2515_RXnBF_OUTPUT 	0

#if SUPPORT_EXTENDED_CANID
#define MCP2515_FILTER_EXTENDED(id)	\
				(uint8_t)  ((uint32_t) (id) >> 21), \
				(uint8_t)((((uint32_t) (id) >> 13) & 0xe0) | (1<<3) | \
					(((uint32_t) (id) >> 16) & 0x3)), \
				(uint8_t)  ((uint32_t) (id) >> 8), \
				(uint8_t)  ((uint32_t) (id))
#endif

#define	MCP2515_FILTER(id) \
			(uint8_t)((uint32_t) id >> 3), \
			(uint8_t)((uint32_t) id << 5), \
			0, \
			0

typedef struct
{
#if SUPPORT_EXTENDED_CANID
  uint32_t id;
  struct
  {
    int rtr:1;
    int extended:1;
  } flags;
#else
  uint16_t id;
  struct
  {
    int rtr:1;
  } flags;
#endif

  uint8_t length;
  uint8_t data[8];

} can_t;

typedef struct
{
  uint8_t rx;
  uint8_t tx;
} can_error_register_t;

typedef enum
{
  LISTEN_ONLY_MODE, LOOPBACK_MODE, NORMAL_MODE, SLEEP_MODE
} can_mode_t;

#ifdef DEBUG_MCP2515
#include "core/debug.h"
#define CANDEBUG(a...)  debug_printf("CAN: " a)
#else
#define CANDEBUG(a...)
#endif

extern bool mcp2515_init(can_bitrate_t bitrate);
extern bool mcp2515_send_message(can_t * message);
extern bool mcp2515_get_message(can_t * message);
extern void mcp2515_set_mode(can_mode_t mode);
extern can_error_register_t mcp2515_read_error_register(void);
extern uint8_t mcp2515_read_error_flags(void);
extern void mcp2515_static_filter(const uint8_t * filter_array);
extern bool mcp2515_check_message(void);
extern void mcp2515_send_tx_fifo(void);
extern uint32_t mcp2515_get_counter_messages_received(void);
extern uint32_t mcp2515_get_counter_messages_send(void);
extern void mcp2515_reset_counter(void);

#ifdef DEBUG_MCP2515
extern void mcp2515_print_message(const can_t * message);
#endif

#ifdef MCP2515_INTERRUPT
extern bool mcp2515_check_rx_fifo(void);
#endif

#if MCP2515_RXnBF_OUTPUT
extern void mcp2515_rx0bf(uint8_t level);
extern void mcp2515_rx1bf(uint8_t level);
#endif

#endif
