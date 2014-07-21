/*
 * Remote Infrared-Multiprotokoll-Decoder
 *
 * for additional information please
 * see https://www.mikrocontroller.net/articles/Remote_IRMP
 *
 * Copyright (c) 2014 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __REMOTE_IRMP_STATE_H
#define __REMOTE_IRMP_STATE_H

#include <stdint.h>
#include "protocols/uip/uip-conf.h"

struct remote_irmp_connection_state_t
{
  enum {
    REMOTE_IRMP_IDLE = 0,
    REMOTE_IRMP_SEND,
    REMOTE_IRMP_RECEIVE,
    REMOTE_IRMP_PING
  } state;
  uip_ipaddr_t reply_addr;
  uint16_t reply_port;
  uint8_t timeout;
};

#endif /* __REMOTE_IRMP_STATE_H */
