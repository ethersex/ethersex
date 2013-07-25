/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef TTY_VT100_TELNET_H
#define TTY_VT100_TELNET_H

#define TELNET_TCP_PORT	23

struct tty_vt100_state_t {
  unsigned send_all	:1;

  /* Pointers to vt100_buf structure, up to which point our peer already
     acked data reception, and up to where it's outstanding ... */
  char *acked, *sent;
};

void tty_vt100_telnet_init (void);

#endif
