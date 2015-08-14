/*
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2015 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <avr/pgmspace.h>

#include "config.h"
#include "protocols/uip/uip.h"


typedef void (*client_return_text_callback_t) (char *, uint8_t);

uip_conn_t *ecmd_sender_send_command_P(uip_ipaddr_t *,
                                       client_return_text_callback_t,
                                       PGM_P, ...);
uip_conn_t *ecmd_sender_send_command(uip_ipaddr_t *,
                                     client_return_text_callback_t,
                                     const char *, ...);

uip_udp_conn_t *uecmd_sender_send_command_P(uip_ipaddr_t *,
                                            client_return_text_callback_t,
                                            PGM_P, ...);
uip_udp_conn_t *uecmd_sender_send_command(uip_ipaddr_t *,
                                          client_return_text_callback_t,
                                          const char *, ...);

#endif /* ECMD_SENDER_NET_H */
