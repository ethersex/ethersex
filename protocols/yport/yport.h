/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef _YPORT_H
#define _YPORT_H


/* The default usart baudrate is 115200 */
#define YPORT_BUFFER_LEN 255

struct yport_buffer {
  uint8_t len;
  uint8_t sent;
  uint8_t data[YPORT_BUFFER_LEN];
};

void yport_init(void);
uint8_t yport_rxstart(uint8_t *data, uint8_t len);

extern struct yport_buffer yport_send_buffer;
extern struct yport_buffer yport_recv_buffer;
#ifdef YPORT_ECMD
extern uint16_t yport_rx_frameerror;
extern uint16_t yport_rx_overflow;
extern uint16_t yport_rx_parityerror;
extern uint16_t yport_rx_bufferfull;
#endif

#endif /* _YPORT_H */
