/*
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
 */

#ifndef ECMD_SENDER_NET_H
#define ECMD_SENDER_NET_H

#include <stdint.h>
typedef void (*client_return_text_callback_t)(char* text, uint8_t len);
#include "protocols/uip/uip.h"
#include <avr/pgmspace.h>

uip_conn_t *ecmd_sender_send_command (uip_ipaddr_t *ipaddr,
				      const char *pgm_data,
				      client_return_text_callback_t callback);
void ecmd_sender_net_main(void);

void uecmd_sender_pgm_send_command (uip_ipaddr_t *ipaddr,
				      PGM_P pgm_data,
				      client_return_text_callback_t callback);
void uecmd_sender_pgm_net_main(void);

void uecmd_sender_send_command (uip_ipaddr_t *ipaddr,
				      char *data,
				      client_return_text_callback_t callback);
void uecmd_sender_net_main(void);

#endif /* ECMD_SENDER_NET_H */
