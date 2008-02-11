/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef _YPORT_H
#define _YPORT_H

/* UART_UBBR is used if teensy is enabled */
#define YPORT_UART_UBRR 10
/* is used if !TEENSY; this ist the baudrate/100 */
#define YPORT_BAUDRATE 1152
#define YPORT_BUFFER_LEN 255

struct yport_buffer {
  uint8_t len;
  uint8_t sent;
  uint8_t data[YPORT_BUFFER_LEN];
};

void yport_init(void);
void yport_rxstart(uint8_t *data, uint8_t len);
/* The baudrate had to be baudrate/100 */
#ifndef TEENSY_SUPPORT
void yport_baudrate(uint16_t baudrate);
#endif

extern struct yport_buffer yport_send_buffer;
extern struct yport_buffer yport_recv_buffer;

#endif /* _YPORT_H */
