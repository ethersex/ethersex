/*
 * Copyright (c) 2013-2017 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __BT_USART_H
#define __BT_USART_H

#include <stdint.h>
#include <stdbool.h>

extern bool bt_init_finished;
extern uint16_t bt_rx_frameerror;
extern uint16_t bt_rx_overflow;
extern uint16_t bt_rx_parityerror;
extern uint16_t bt_rx_bufferfull;
extern uint16_t bt_rx_count;
extern uint16_t bt_tx_count;

void bt_usart_init(const uint8_t, const uint8_t, const bool);
uint8_t bt_usart_free(void);
void bt_usart_putchar(const uint8_t);
uint8_t bt_usart_avail(void);
uint8_t bt_usart_getchar(void);

#endif /* __BT_USART_H */
