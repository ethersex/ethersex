/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef _ZBUS_H
#define _ZBUS_H

#include <stdint.h>
#include "../uip/uip.h"

#define RXTX_PORT PORTC
#define RXTX_DDR  DDRC
#define RXTX_PIN PC2

/* use 19200 baud at 20mhz (see datasheet for other values) */
#define ZBUS_UART_UBRR 64

enum ZBusEscapes {
  ZBUS_START = '0',
  ZBUS_STOP = '1',
};

void zbus_core_init(struct uip_udp_conn *recv_conn);

typedef uint8_t (*zbus_send_byte_callback_t)(void **ctx);

void  zbus_tx_finish(void);
uint8_t zbus_tx_start(zbus_send_byte_callback_t cb, void *ctx);

uint8_t zbus_send_conn_data(struct uip_udp_conn *conn);


#endif /* _ZBUS_H */
