/*
 *
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef UIP_OPENVPN_H
#define UIP_OPENVPN_H

#include "config.h"
#ifdef OPENVPN_SUPPORT

#include "uip-conf.h"

void openvpn_handle_udp(void);
void openvpn_init (void);
void openvpn_process_out (void);
void openvpn_txstart (void);

/* The port number to use for OpenVPN. */
#define OPENVPN_PORT CONF_OPENVPN_PORT

struct openvpn_connection_state_t {
  uint32_t next_seqno;
  uint32_t seen_seqno;
  uint32_t seen_timestamp;
};

#endif /* OPENVPN_SUPPORT */
#endif /* UIP_OPENVPN_H */
