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

#ifndef _EMS_H
#define _EMS_H

#define OUR_EMS_ADDRESS     0x0b
#define EMS_REQUEST_MASK    0x80
#define EMS_MSG_TYPE_RESP   0xff
#define EMS_RESPONSE_OK     0x01
#define EMS_RESPONSE_FAIL   0x04

#define EMS_UART_INPUT_BUFSIZE 16
#define EOP_BYTE(value) (value >> 3)
#define EOP_BIT(value) (1 << (value & 0x7))
struct ems_uart_input_buffer {
  uint8_t data[EMS_UART_INPUT_BUFSIZE];
  uint8_t count;
  uint8_t eop[EMS_UART_INPUT_BUFSIZE / 8];
};

struct ems_stats {
  uint32_t total_bytes;
  uint32_t good_bytes;
  uint32_t dropped_bytes;
  uint32_t onebyte_packets;
  uint32_t onebyte_own_packets;
  uint32_t onebyte_ack_packets;
  uint32_t onebyte_nack_packets;
  uint32_t good_packets;
  uint32_t bad_packets;
  uint32_t dropped_packets;
  uint32_t buffer_overflow;
  uint8_t max_fill;
};

struct ems_buffer {
  uint16_t len;
  uint16_t sent;
  uint8_t addr_byte; /* space for address byte when calculating TX checksum */
  uint8_t data[EMS_BUFFER_LEN];
};

void ems_init(void);
void ems_process(void);
void ems_periodic_timeout(void);
uint8_t ems_process_txdata(uint8_t *data, uint16_t len);
uint8_t ems_calc_checksum(const uint8_t *buffer, uint8_t size);
uint8_t ems_net_connected(void);
void ems_uart_got_response(void);
void ems_add_source_to_eop(uint8_t source);

void ems_uart_init(void);
void ems_uart_periodic(void);

void ems_net_init(void);
void ems_net_main(void);

#define FRAMEEND _BV(0)
#define ERROR    _BV(1)
void ems_uart_process_input_byte(uint8_t data, uint8_t status);

#define LED_TX       0
#define LED_RX_OK    1
#define LED_RX_FAIL  2
#define EMS_NUM_LEDS 3
void ems_set_led(uint8_t led, uint8_t enable, uint8_t timeout /* x 100ms */);

extern uint8_t ems_poll_address;
extern struct ems_uart_input_buffer ems_input_buffer;
extern struct ems_buffer ems_send_buffer;
extern struct ems_buffer ems_recv_buffer;
#ifdef EMS_DEBUG_STATS
extern struct ems_stats ems_stats_buffer;
#define UPDATE_STATS(value,count) (ems_stats_buffer. value += count)
#else
#define UPDATE_STATS(...)
#endif

#include "config.h"
#if defined (EMS_PROTO_DEBUG) || defined(EMS_IO_DEBUG) || defined(EMS_ERROR_DEBUG)
# include "core/debug.h"
#endif

#ifdef EMS_PROTO_DEBUG
# define EMSPROTODEBUG(a...)  debug_printf("ems: " a)
#else
# define EMSPROTODEBUG(a...)
#endif

#ifdef EMS_IO_DEBUG
# define EMSIODEBUG(a...)  debug_printf("ems: " a)
#else
# define EMSIODEBUG(a...)
#endif

#ifdef EMS_ERROR_DEBUG
# define EMSERRORDEBUG(a...)  debug_printf("ems: " a)
#else
# define EMSERRORDEBUG(a...)
#endif

#endif /* _EMS_H */
